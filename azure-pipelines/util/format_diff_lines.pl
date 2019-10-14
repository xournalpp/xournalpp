#/usr/bin/perl

# Given the diff output on stdin, run clang-format on the input hunks.
#
# Author: Bryan Tan (Technius)

use strict;

# Table mapping changed files to array of hunks.
my %changed_files;
# Array of hunks in the current file. Only stores the new location.
my @hunks;
# Current file
my $current_file;

my @CLANG_FMT_CMDS = ("clang-format", "clang-format-8");
my $clang_format_cmd = "";
for my $c (@CLANG_FMT_CMDS) {
	if (`$c --version`) {
		$clang_format_cmd = $c;
		last;
	}
}

if ($clang_format_cmd eq "") {
	printf "$0: clang-format not found\n";
	exit 1;
}

# Push the pending hunks to the changed file table.
sub push_changes {
	if (scalar @hunks > 0) {
		$changed_files{$current_file} = [@hunks];
		@hunks = ();
	}
}

while (my $line = <STDIN>) {
	if ($line =~ /^diff .* a\/(?<old_file>.*) b\/(?<new_file>.*)$/) {
		# New file
		push_changes;
		$current_file = $+{new_file};
	} elsif ($line =~ /^@@ -([0-9]+)(,([0-9]+))? \+(?<new_pos>[0-9]+)(,(?<new_len>[0-9]+))? @@/) {
		my $new_len = $+{new_len};
		$new_len = 1 if ($new_len eq "");
		push @hunks, [$+{new_pos}, $new_len];
	}
}
push_changes;

# Now, run clang format on the changed lines.
while (my ($file_name, $hunks_ref) = each %changed_files) {
	my @ranges = ();
	foreach my $hunk_ref (@{$hunks_ref}) {
		my $start_pos = @{$hunk_ref}[0];
		my $line_count = @{$hunk_ref}[1];
		my $end_pos = $start_pos + $line_count - 1;
		if ($line_count > 0) {
			push @ranges, [$start_pos, $end_pos];
		}
	}
	my $ranges_str = join(", ", map { "@{$_}[0]-@{$_}[1]" } @ranges);
	my $lines_arg = join(" ", map { "-lines=@{$_}[0]:@{$_}[1]" } @ranges);
	printf "Running $clang_format_cmd on %s, lines %s (inclusive)\n", $file_name, $ranges_str;
	`$clang_format_cmd -i -style=file $lines_arg $file_name`;
	die "Failed to run $clang_format_cmd: exit code $?" if $? != 0;
}
