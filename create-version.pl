eval 'exec perl -w -S $0 ${1+"$@"}'
    if 0;

use strict;

if (scalar @ARGV < 3) { exit(1); }
my $input = shift;
my $output = shift;
my $src = shift;
my $incSvn = 1;
if (scalar @ARGV > 0) { $incSvn = shift; }
my $incDate = 1;
if (scalar @ARGV > 0) { $incDate = shift; }

my $svnversion = `cd $src && svnversion`;
chomp $svnversion;
my $date = `date`;
chomp $date;

open(INPUT,"<$input") or die "Unable to open input file ($input): $!\n";
open(OUTPUT,">$output") or die "Unable to open output file ($output): $!\n";
while (<INPUT>) {
  if ($incSvn) {
    s%\/\*SVN_VERSION_STRING\*\/%\#define SVN_VERSION_STRING \"$svnversion\"%;
  }
  if ($incDate) {
    s%\/\*DATE_STRING\*\/%\#define DATE_STRING \"$date\"%;
  }
  print OUTPUT $_;
}
close INPUT;
close OUTPUT;

