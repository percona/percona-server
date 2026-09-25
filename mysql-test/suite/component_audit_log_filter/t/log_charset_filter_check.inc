perl;
  use strict;
  use warnings;
  use Encode qw(decode FB_CROAK);
  use JSON;
  open my $f, '<:raw', "$ENV{charset_dir}/$ENV{charset_closed}" or die $!;
  my $bytes = do { local $/; <$f> };
  my $records = JSON->new->decode(decode('UTF-8', $bytes, FB_CROAK));
  my $expected = decode('UTF-8', $ENV{charset_query});
  my @queries = map { $_->{$ENV{charset_class} . '_data'}->{query} }
                grep { $_->{class} eq $ENV{charset_class} } @$records;
  # Client initialization can also generate events in the unconditional digest
  # case; only inspect the fixture statements, identifiable by their alias.
  @queries = grep { / AS (?:c$|c FROM|caf|`caf)/ } @queries;
  die 'Wrong event count' unless @queries == $ENV{charset_expected_count};
  die 'Missing exact converted query' unless grep { $_ eq $expected } @queries;
  if ($ENV{charset_expect_digest}) {
    die 'Digest was decoded in client charset or print condition changed' unless
      1 == grep { $_ eq "SELECT ? AS `caf\x{e9}` FROM `charset_filter_data`" } @queries;
  }
EOF
