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
	foreach my $hunk_ref (@{$hunks_ref}) {
		my $start_pos = @{$hunk_ref}[0];
		my $line_count = @{$hunk_ref}[1];
		my $end_pos = $start_pos + $line_count - 1;
		if ($line_count > 0) {
			printf "Running clang-format-8 on %s, lines %i-%i\n", $file_name, $start_pos, $end_pos;
			`clang-format-8 -i -style=file -lines=$start_pos:$end_pos $file_name`;
			die "Failed to run clang-format-8: exit code $?" if $? != 0;
		}
	}
}
