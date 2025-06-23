#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

#include <dlfcn.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <path_to_library.so>\n";
    return 1;
  }

  const char *lib_path = argv[1];
  auto dl_closer{[](void *dl_handle) {
    if (dl_handle != nullptr) dlclose(dl_handle);
  }};
  using handle_guard = std::unique_ptr<void, decltype(dl_closer)>;
  handle_guard handle{dlopen(lib_path, RTLD_NOW), std::move(dl_closer)};
  if (!handle) {
    std::cerr << "dlopen() failed: " << dlerror() << '\n';
    return EXIT_FAILURE;
  }

  std::cout << "dlopen() succeeded: " << lib_path << '\n';
  return EXIT_SUCCESS;
}
