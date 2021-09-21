#include "./enc_env_encryption_myrocks.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <vector>
#include "env/composite_env_wrapper.h"

#include "file/filename.h"
#include "monitoring/perf_context_imp.h"
#include "rocksdb/convenience.h"
#include "rocksdb/io_status.h"
#include "rocksdb/system_clock.h"
#include "util/aligned_buffer.h"
#include "util/coding.h"
#include "util/random.h"
#include "util/string_util.h"

namespace myrocks {
using namespace rocksdb;

namespace {
// MyRocksEncryptedFileSystemImpl implements an FileSystemWrapper that adds
// encryption to files stored on disk.
// This implementation is heavily based on EncryptedFileSystemImpl from rocksdb
// however, we need the logic to be a bit different + we need to add master
// key encryption support. Morover EncryptedFileSystemImpl is private to RocksDB
// so we have no chance to extend it whatsoever.
// That's why we provide extended implementation in MyRocks layer.
// However, let's keep it similar to rocksdb::EncryptedFileSystemImpl for now
// for easy changes tracking in the future.
class MyRocksEncryptedFileSystemImpl : public MyRocksEncryptedFileSystem {
 public:
  const char *Name() const override { return "MyRocksEncryptedFS"; }
  // Returns the raw encryption provider that should be used to write the input
  // encrypted file.  If there is no such provider, NotFound is returned.
  IOStatus GetWritableProvider(const std::string & /*fname*/,
                               EncryptionProvider **result) {
    if (provider_) {
      *result = provider_.get();
      return IOStatus::OK();
    } else {
      *result = nullptr;
      return IOStatus::NotFound("No WriteProvider specified");
    }
  }

  // Returns the raw encryption provider that should be used to read the input
  // encrypted file.  If there is no such provider, NotFound is returned.
  IOStatus GetReadableProvider(const std::string & /*fname*/,
                               EncryptionProvider **result) {
    if (provider_) {
      *result = provider_.get();
      return IOStatus::OK();
    } else {
      *result = nullptr;
      return IOStatus::NotFound("No Provider specified");
    }
  }

  // Creates a CipherStream for the underlying file/name using the options
  // If a writable provider is found and encryption is enabled, uses
  // this provider to create a cipher stream.
  // @param fname         Name of the writable file
  // @param underlying    The underlying "raw" file
  // @param options       Options for creating the file/cipher
  // @param prefix_length Returns the length of the encryption prefix used for
  // this file
  // @param stream        Returns the cipher stream to use for this file if it
  // should be encrypted
  // @return OK on success, non-OK on failure.
  template <class TypeFile>
  IOStatus CreateWritableCipherStream(
      const std::string &fname, const std::unique_ptr<TypeFile> &underlying,
      const FileOptions &options, size_t *prefix_length, bool createPrefix,
      std::unique_ptr<BlockAccessCipherStream> *stream, IODebugContext *dbg) {
    EncryptionProvider *provider = nullptr;
    *prefix_length = 0;
    IOStatus status = GetWritableProvider(fname, &provider);
    if (!status.ok()) {
      return status;
    } else if (provider != nullptr) {
      // Initialize & write prefix (if needed)
      AlignedBuffer buffer;
      Slice prefix;
      *prefix_length = provider->GetPrefixLength();
      if (*prefix_length > 0) {
        if (createPrefix) {
          // Initialize prefix
          buffer.Alignment(underlying->GetRequiredBufferAlignment());
          buffer.AllocateNewBuffer(*prefix_length);
          status = status_to_io_status(provider->CreateNewPrefix(
              fname, buffer.BufferStart(), *prefix_length));
          if (status.ok()) {
            buffer.Size(*prefix_length);
            prefix = Slice(buffer.BufferStart(), buffer.CurrentSize());
            // Write prefix
            status = underlying->Append(prefix, options.io_options, dbg);
          }
          if (!status.ok()) {
            return status;
          }
        } else {
          // Read prefix, but underlying is write only, so need to open as
          // readable
          rocksdb::EnvOptions soptions;
          std::unique_ptr<FSSequentialFile> underlyingSR;
          status = FileSystemWrapper::NewSequentialFile(fname, soptions,
                                                        &underlyingSR, dbg);
          if (!status.ok()) {
            return status;
          }

          buffer.Alignment(underlyingSR->GetRequiredBufferAlignment());
          buffer.AllocateNewBuffer(*prefix_length);
          status = underlyingSR->Read(*prefix_length, options.io_options,
                                      &prefix, buffer.BufferStart(), dbg);
          if (!status.ok()) {
            return status;
          }
          buffer.Size(*prefix_length);
        }
      }
      // Create cipher stream
      status = status_to_io_status(
          provider->CreateCipherStream(fname, options, prefix, stream));
    }
    return status;
  }

  template <class TypeFile>
  IOStatus CreateWritableEncryptedFile(const std::string &fname,
                                       std::unique_ptr<TypeFile> &underlying,
                                       const FileOptions &options,
                                       bool createPrefix,
                                       std::unique_ptr<TypeFile> *result,
                                       IODebugContext *dbg) {
    // Create cipher stream
    std::unique_ptr<BlockAccessCipherStream> stream;
    size_t prefix_length;
    IOStatus status = CreateWritableCipherStream(
        fname, underlying, options, &prefix_length, createPrefix, &stream, dbg);
    if (status.ok()) {
      if (stream) {
        result->reset(new EncryptedWritableFile(
            std::move(underlying), std::move(stream), prefix_length));
      } else {
        result->reset(underlying.release());
      }
    }
    return status;
  }

  // Creates a CipherStream for the underlying file/name using the options
  // If a writable provider is found and encryption is enabled, uses
  // this provider to create a cipher stream.
  // @param fname         Name of the writable file
  // @param underlying    The underlying "raw" file
  // @param options       Options for creating the file/cipher
  // @param prefix_length Returns the length of the encryption prefix used for
  // this file
  // @param stream        Returns the cipher stream to use for this file if it
  // should be encrypted
  // @return OK on success, non-OK on failure.
  template <class TypeFile>
  IOStatus CreateRandomWriteCipherStream(
      const std::string &fname, const std::unique_ptr<TypeFile> &underlying,
      const FileOptions &options, size_t *prefix_length,
      std::unique_ptr<BlockAccessCipherStream> *stream, IODebugContext *dbg) {
    EncryptionProvider *provider = nullptr;
    *prefix_length = 0;
    IOStatus io_s = GetWritableProvider(fname, &provider);
    if (!io_s.ok()) {
      return io_s;
    } else if (provider != nullptr) {
      // Initialize & write prefix (if needed)
      AlignedBuffer buffer;
      Slice prefix;
      *prefix_length = provider->GetPrefixLength();
      if (*prefix_length > 0) {
        // Initialize prefix
        buffer.Alignment(underlying->GetRequiredBufferAlignment());
        buffer.AllocateNewBuffer(*prefix_length);
        io_s = status_to_io_status(provider->CreateNewPrefix(
            fname, buffer.BufferStart(), *prefix_length));
        if (io_s.ok()) {
          buffer.Size(*prefix_length);
          prefix = Slice(buffer.BufferStart(), buffer.CurrentSize());
          // Write prefix
          io_s = underlying->Write(0, prefix, options.io_options, dbg);
        }
        if (!io_s.ok()) {
          return io_s;
        }
      }
      // Create cipher stream
      io_s = status_to_io_status(
          provider->CreateCipherStream(fname, options, prefix, stream));
    }
    return io_s;
  }

  // Creates a CipherStream for the underlying file/name using the options
  // If a readable provider is found and the file is encrypted, uses
  // this provider to create a cipher stream.
  // @param fname         Name of the writable file
  // @param underlying    The underlying "raw" file
  // @param options       Options for creating the file/cipher
  // @param prefix_length Returns the length of the encryption prefix used for
  // this file
  // @param stream        Returns the cipher stream to use for this file if it
  // is encrypted
  // @return OK on success, non-OK on failure.
  template <class TypeFile>
  IOStatus CreateSequentialCipherStream(
      const std::string &fname, const std::unique_ptr<TypeFile> &underlying,
      const FileOptions &options, size_t *prefix_length,
      std::unique_ptr<BlockAccessCipherStream> *stream, IODebugContext *dbg) {
    // Read prefix (if needed)
    AlignedBuffer buffer;
    Slice prefix;
    *prefix_length = provider_->GetPrefixLength();
    if (*prefix_length > 0) {
      // Read prefix
      buffer.Alignment(underlying->GetRequiredBufferAlignment());
      buffer.AllocateNewBuffer(*prefix_length);
      IOStatus status = underlying->Read(*prefix_length, options.io_options,
                                         &prefix, buffer.BufferStart(), dbg);
      if (!status.ok()) {
        return status;
      }
      buffer.Size(*prefix_length);
    }
    return status_to_io_status(
        provider_->CreateCipherStream(fname, options, prefix, stream));
  }

  // Creates a CipherStream for the underlying file/name using the options
  // If a readable provider is found and the file is encrypted, uses
  // this provider to create a cipher stream.
  // @param fname         Name of the writable file
  // @param underlying    The underlying "raw" file
  // @param options       Options for creating the file/cipher
  // @param prefix_length Returns the length of the encryption prefix used for
  // this file
  // @param stream        Returns the cipher stream to use for this file if it
  // is encrypted
  // @return OK on success, non-OK on failure.
  template <class TypeFile>
  IOStatus CreateRandomReadCipherStream(
      const std::string &fname, const std::unique_ptr<TypeFile> &underlying,
      const FileOptions &options, size_t *prefix_length,
      std::unique_ptr<BlockAccessCipherStream> *stream, IODebugContext *dbg,
      bool threadSafeStream = false) {
    // Read prefix (if needed)
    AlignedBuffer buffer;
    Slice prefix;
    *prefix_length = provider_->GetPrefixLength();
    if (*prefix_length > 0) {
      // Read prefix
      buffer.Alignment(underlying->GetRequiredBufferAlignment());
      buffer.AllocateNewBuffer(*prefix_length);
      IOStatus status = underlying->Read(0, *prefix_length, options.io_options,
                                         &prefix, buffer.BufferStart(), dbg);
      if (!status.ok()) {
        return status;
      }
      buffer.Size(*prefix_length);
    }

    if (threadSafeStream) {
      return status_to_io_status(provider_->CreateThreadSafeCipherStream(
          fname, options, prefix, stream));
    } else {
      return status_to_io_status(
          provider_->CreateCipherStream(fname, options, prefix, stream));
    }
  }

 public:
  MyRocksEncryptedFileSystemImpl(
      const std::shared_ptr<FileSystem> &base,
      const std::shared_ptr<MyRocksEncryptionProvider> &provider,
      bool encryptNewFiles)
      : MyRocksEncryptedFileSystem(base),
        provider_(provider),
        encrypt_new_files_(encryptNewFiles) {}

  Status Init(const std::string dir) override {
    IOOptions io_opts;
    IODebugContext dbg;
    std::vector<std::string> files;

    if (!GetChildren(dir, io_opts, &files, &dbg).ok()) {
      return Status::OK();  // if the directory does not exist there are no
                            // files
    }

    uint64_t number = 0;
    FileType type = kInfoLogFile;
    for (const std::string &file : files) {
      if (!ParseFileName(file, &number, &type)) {
        continue;
      }

      FeedEncryptionProvider(dir + "/" + file);
    }

    return Status::OK();
  }

  Status RotateFileMasterEncryptionKey(const std::string &fname) {
    rocksdb::EnvOptions soptions;
    IODebugContext dbg;

    std::unique_ptr<FSRandomRWFile> underlying;
    auto io_status =
        FileSystemWrapper::NewRandomRWFile(fname, soptions, &underlying, &dbg);
    if (!io_status.ok()) {
      return io_status;
    }
    auto encPrefix = provider_->GetMarker();
    std::vector<char> buffer;
    buffer.reserve(provider_->GetPrefixLength());
    Slice prefix;
    io_status = underlying->Read(0, provider_->GetPrefixLength(), IOOptions(),
                                 &prefix, buffer.data(), nullptr);
    if (!io_status.ok()) {
      return io_status;
    }
    auto encrypted = (encPrefix.compare(0, encPrefix.size(), prefix.data(),
                                        encPrefix.size()) == 0);

    if (encrypted) {
      auto reencryptStatus = provider_->ReencryptPrefix(prefix);

      // now store it back
      if (reencryptStatus == Status::OK()) {
        // Write prefix
        io_status = underlying->Write(0, prefix, IOOptions(), &dbg);
      }
    }
    return io_status;
  }

  Status RotateEncryptionMasterKey(const std::string dir) override {
    IOOptions io_opts;
    IODebugContext dbg;
    std::vector<std::string> files;

    if (!GetChildren(dir, io_opts, &files, &dbg).ok()) {
      return Status::OK();  // if the directory does not exist there are no
                            // files
    }

    uint64_t number = 0;
    FileType type = kInfoLogFile;
    Status status = Status::OK();
    for (const std::string &file : files) {
      if (!ParseFileName(file, &number, &type)) {
        continue;
      }

      status = RotateFileMasterEncryptionKey(dir + "/" + file);
      if (status != Status::OK()) {
        fprintf(stderr, "MK rotation for file %s failed\n",
                (dir + "/" + file).c_str());
        break;
      }
    }
    return status;
  }

  IOStatus FeedEncryptionProvider(const std::string &fname) {
    rocksdb::EnvOptions soptions;
    IODebugContext dbg;

    std::unique_ptr<FSSequentialFile> underlying;
    auto status = FileSystemWrapper::NewSequentialFile(fname, soptions,
                                                       &underlying, &dbg);
    if (!status.ok()) {
      return status;
    }
    auto encPrefix = provider_->GetMarker();
    std::vector<char> buffer;
    buffer.reserve(provider_->GetPrefixLength());
    Slice prefix;
    status = underlying->Read(provider_->GetPrefixLength(), IOOptions(),
                              &prefix, buffer.data(), nullptr);
    if (!status.ok()) {
      return status;
    }
    auto encrypted = (encPrefix.compare(0, encPrefix.size(), prefix.data(),
                                        encPrefix.size()) == 0);

    if (encrypted) {
      provider_->Feed(prefix);
    }

    return status_to_io_status(Status::OK());
  }

  IOStatus IsFileEncrypted(const std::string &fname, bool *result,
                           IODebugContext *dbg) {
    rocksdb::EnvOptions soptions;
    std::unique_ptr<FSSequentialFile> underlying;
    auto status =
        FileSystemWrapper::NewSequentialFile(fname, soptions, &underlying, dbg);
    if (!status.ok()) {
      return status;
    }
    auto encPrefix = provider_->GetMarker();
    std::vector<char> space;
    space.reserve(encPrefix.size());
    Slice fragment;
    status = underlying->Read(encPrefix.size(), IOOptions(), &fragment,
                              space.data(), nullptr);
    if (!status.ok()) {
      return status;
    }
    *result = (encPrefix.compare(0, encPrefix.size(), fragment.data(),
                                 fragment.size()) == 0);

    return status_to_io_status(Status::OK());
  }

  Status AddCipher(const std::string &descriptor, const char *cipher,
                   size_t len, bool for_write) override {
    provider_->AddCipher(descriptor, cipher, len, for_write);
    return Status::OK();
  }

  // NewSequentialFile opens a file for sequential reading.
  IOStatus NewSequentialFile(const std::string &fname,
                             const FileOptions &options,
                             std::unique_ptr<FSSequentialFile> *result,
                             IODebugContext *dbg) override {
    result->reset();
    if (options.use_mmap_reads) {
      return IOStatus::InvalidArgument();
    }
    // Open file using underlying Env implementation
    std::unique_ptr<FSSequentialFile> underlying;
    auto status =
        FileSystemWrapper::NewSequentialFile(fname, options, &underlying, dbg);
    if (!status.ok()) {
      return status;
    }
    uint64_t file_size;
    status = FileSystemWrapper::GetFileSize(fname, options.io_options,
                                            &file_size, dbg);
    if (!status.ok()) {
      return status;
    }
    if (!file_size) {
      *result = std::move(underlying);
      return status;
    }

    bool encrypted = false;
    IsFileEncrypted(fname, &encrypted, dbg);
    if (!encrypted) {
      result->reset(underlying.release());
      return status;
    }

    // Create cipher stream
    std::unique_ptr<BlockAccessCipherStream> stream;
    size_t prefix_length;
    status = CreateSequentialCipherStream(fname, underlying, options,
                                          &prefix_length, &stream, dbg);
    if (status.ok()) {
      result->reset(new EncryptedSequentialFile(
          std::move(underlying), std::move(stream), prefix_length));
    }
    return status;
  }

  // NewRandomAccessFile opens a file for random read access.
  IOStatus NewRandomAccessFile(const std::string &fname,
                               const FileOptions &options,
                               std::unique_ptr<FSRandomAccessFile> *result,
                               IODebugContext *dbg) override {
    result->reset();
    if (options.use_mmap_reads) {
      return IOStatus::InvalidArgument();
    }
    // Open file using underlying Env implementation
    std::unique_ptr<FSRandomAccessFile> underlying;
    auto status = FileSystemWrapper::NewRandomAccessFile(fname, options,
                                                         &underlying, dbg);
    if (!status.ok()) {
      return status;
    }

    bool encrypted = false;
    IsFileEncrypted(fname, &encrypted, dbg);
    if (!encrypted) {
      result->reset(underlying.release());
      return status;
    }

    std::unique_ptr<BlockAccessCipherStream> stream;
    size_t prefix_length;
    status = CreateRandomReadCipherStream(fname, underlying, options,
                                          &prefix_length, &stream, dbg, true);
    if (status.ok()) {
      if (stream) {
        result->reset(new EncryptedRandomAccessFile(
            std::move(underlying), std::move(stream), prefix_length));
      } else {
        result->reset(underlying.release());
      }
    }
    return status;
  }

  // NewWritableFile opens a file for sequential writing.
  IOStatus NewWritableFile(const std::string &fname, const FileOptions &options,
                           std::unique_ptr<FSWritableFile> *result,
                           IODebugContext *dbg) override {
    result->reset();
    if (options.use_mmap_writes) {
      return IOStatus::InvalidArgument();
    }
    // Open file using underlying Env implementation
    std::unique_ptr<FSWritableFile> underlying;
    IOStatus status =
        FileSystemWrapper::NewWritableFile(fname, options, &underlying, dbg);
    if (!status.ok()) {
      return status;
    }

    // file should not be encrypted
    if (!encrypt_new_files_) {
      result->reset(underlying.release());
      return status;
    }

    return CreateWritableEncryptedFile(fname, underlying, options, true, result,
                                       dbg);
  }

  // Create an object that writes to a new file with the specified
  // name.  Deletes any existing file with the same name and creates a
  // new file. (KH: this is not true. see how posix layer works)
  // On success, stores a pointer to the new file in
  // *result and returns OK.  On failure stores nullptr in *result and
  // returns non-OK.
  //
  // The returned file will only be accessed by one thread at a time.
  IOStatus ReopenWritableFile(const std::string &fname,
                              const FileOptions &options,
                              std::unique_ptr<FSWritableFile> *result,
                              IODebugContext *dbg) override {
    result->reset();
    if (options.use_mmap_writes) {
      return IOStatus::InvalidArgument();
    }

    bool isNewFile = !FileExists(fname, options.io_options, dbg).ok();

    // Open file using underlying Env implementation
    std::unique_ptr<FSWritableFile> underlying;
    IOStatus status =
        FileSystemWrapper::ReopenWritableFile(fname, options, &underlying, dbg);
    if (!status.ok()) {
      return status;
    }

    // file exists and is not encrypted
    if (!isNewFile) {
      bool encrypted = false;
      IsFileEncrypted(fname, &encrypted, dbg);
      if (!encrypted) {
        result->reset(underlying.release());
        return status;
      }
    }

    // file does not exist and is not encrypted
    if (isNewFile && !encrypt_new_files_) {
      result->reset(underlying.release());
      return status;
    }

    // here we go if we need to create encrypted file (new one, or wrap existing
    // one)
    return CreateWritableEncryptedFile(fname, underlying, options, isNewFile,
                                       result, dbg);
  }

  // Reuse an existing file by renaming it and opening it as writable.
  IOStatus ReuseWritableFile(const std::string &fname,
                             const std::string &old_fname,
                             const FileOptions &options,
                             std::unique_ptr<FSWritableFile> *result,
                             IODebugContext *dbg) override {
    result->reset();
    if (options.use_mmap_writes) {
      return IOStatus::InvalidArgument();
    }
    // Open file using underlying Env implementation
    std::unique_ptr<FSWritableFile> underlying;
    auto status = FileSystemWrapper::ReuseWritableFile(
        fname, old_fname, options, &underlying, dbg);
    if (!status.ok()) {
      return status;
    }

    // file should not be encrypted
    if (!encrypt_new_files_) {
      result->reset(underlying.release());
      return status;
    }

    return CreateWritableEncryptedFile(fname, underlying, options, true, result,
                                       dbg);
  }

  // Open `fname` for random read and write, if file doesn't exist the file
  // will be created.  On success, stores a pointer to the new file in
  // *result and returns OK.  On failure returns non-OK.
  //
  // The returned file will only be accessed by one thread at a time.
  IOStatus NewRandomRWFile(const std::string &fname, const FileOptions &options,
                           std::unique_ptr<FSRandomRWFile> *result,
                           IODebugContext *dbg) override {
    result->reset();
    if (options.use_mmap_reads || options.use_mmap_writes) {
      return IOStatus::InvalidArgument();
    }
    // Check file exists
    bool isNewFile = !FileExists(fname, options.io_options, dbg).ok();

    // Open file using underlying Env implementation
    std::unique_ptr<FSRandomRWFile> underlying;
    auto status =
        FileSystemWrapper::NewRandomRWFile(fname, options, &underlying, dbg);
    if (!status.ok()) {
      return status;
    }

    // file exists and is unencrypted
    if (!isNewFile) {
      bool encrypted = false;
      IsFileEncrypted(fname, &encrypted, dbg);
      if (!encrypted) {
        result->reset(underlying.release());
        return status;
      }
    }

    // file does not exist and it has to be not encrypted
    if (isNewFile && !encrypt_new_files_) {
      result->reset(underlying.release());
      return status;
    }

    // Here we go if the file is encrypted. The new one or existing one.
    // Create cipher stream
    std::unique_ptr<BlockAccessCipherStream> stream;
    size_t prefix_length = 0;
    if (!isNewFile) {
      // File already exists, read prefix
      status = CreateRandomReadCipherStream(fname, underlying, options,
                                            &prefix_length, &stream, dbg);
    } else {
      status = CreateRandomWriteCipherStream(fname, underlying, options,
                                             &prefix_length, &stream, dbg);
    }
    if (status.ok()) {
      if (stream) {
        result->reset(new EncryptedRandomRWFile(
            std::move(underlying), std::move(stream), prefix_length));
      } else {
        result->reset(underlying.release());
      }
    }
    return status;
  }

  // Store in *result the attributes of the children of the specified
  // directory.
  // In case the implementation lists the directory prior to iterating the
  // files
  // and files are concurrently deleted, the deleted files will be omitted
  // from
  // result.
  // The name attributes are relative to "dir".
  // Original contents of *results are dropped.
  // Returns OK if "dir" exists and "*result" contains its children.
  //         NotFound if "dir" does not exist, the calling process does not
  //         have
  //                  permission to access "dir", or if "dir" is invalid.
  //         IOError if an IO Error was encountered
  IOStatus GetChildrenFileAttributes(const std::string &dir,
                                     const IOOptions &options,
                                     std::vector<FileAttributes> *result,
                                     IODebugContext *dbg) override {
    auto status =
        FileSystemWrapper::GetChildrenFileAttributes(dir, options, result, dbg);
    if (!status.ok()) {
      return status;
    }
    for (auto it = std::begin(*result); it != std::end(*result); ++it) {
      // assert(it->size_bytes >= prefixLength);
      //  breaks env_basic_test when called on directory containing
      //  directories
      // which makes subtraction of prefixLength worrisome since
      // FileAttributes does not identify directories
      bool encrypted = false;
      IsFileEncrypted(it->name, &encrypted, dbg);
      if (!encrypted) {
        continue;
      }

      EncryptionProvider *provider;
      status = GetReadableProvider(it->name, &provider);
      if (!status.ok()) {
        return status;
      } else if (provider != nullptr) {
        it->size_bytes -= provider->GetPrefixLength();
      }
    }
    return IOStatus::OK();
  }

  // Store the size of fname in *file_size.
  IOStatus GetFileSize(const std::string &fname, const IOOptions &options,
                       uint64_t *file_size, IODebugContext *dbg) override {
    auto status =
        FileSystemWrapper::GetFileSize(fname, options, file_size, dbg);
    if (!status.ok() || !(*file_size)) {
      return status;
    }

    bool encrypted = false;
    IsFileEncrypted(fname, &encrypted, dbg);
    if (!encrypted) {
      return status;
    }

    EncryptionProvider *provider;
    status = GetReadableProvider(fname, &provider);
    if (provider != nullptr && status.ok()) {
      size_t prefixLength = provider->GetPrefixLength();
      assert(*file_size >= prefixLength);
      *file_size -= prefixLength;
    }
    return status;
  }

 private:
  std::shared_ptr<MyRocksEncryptionProvider> provider_;
  bool encrypt_new_files_;
};
}  // namespace

std::shared_ptr<MyRocksEncryptedFileSystem> NewEncryptedFS(
    const std::shared_ptr<FileSystem> &base,
    const std::shared_ptr<MyRocksEncryptionProvider> &provider,
    bool encryptNewFiles, const std::string &dir) {
  auto res = std::make_shared<MyRocksEncryptedFileSystemImpl>(base, provider,
                                                              encryptNewFiles);
  res->Init(dir);
  return res;
}

}  // namespace myrocks
