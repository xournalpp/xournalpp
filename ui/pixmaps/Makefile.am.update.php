<?php

$entries = file('Makefile.am');

$contents = '';

$dynamicContent = false;

foreach ($entries as $e) {
	$t = trim($e);
	if ('## UPDATE_FILE_LIST_END' == $t) {

		$contents .= "\npixmap_DATA = \\\n";

		if ($handle = opendir('.')) {
			while (false !== ($entry = readdir($handle))) {
				if ($entry[0] == '.') {
					continue;
				}

				$ext = strtolower(substr($entry, -4));

				if ($ext == '.png' || $ext == '.svg') {
					$contents .= "\t$entry \\\n";
				}
			}
			closedir($handle);

			$contents = substr($contents, 0, -3);
			$contents .= "\n\n";
		}

		$dynamicContent = false;
	}
	if ($dynamicContent) {
		continue;
	}

	if ('## UPDATE_FILE_LIST_START' == $t) {
		$dynamicContent = true;
	}


	$contents .= $e;
}

file_put_contents('Makefile.am', $contents);



