<?php


$dontFilterWitespace = false;

$c = 0;

$dir = './src/';

function countline($dir) {
global $dontFilterWitespace;

	$c = 0;
	if (is_dir($dir)) {
		if ($dh = opendir($dir)) {
			while (($file = readdir($dh)) !== false) {
				if(substr($file, -4) == '.cpp') {
					if($dontFilterWitespace) {
						$c += count(file($dir . $file));
					} else {
						foreach(file($dir . $file) as $l) {
							if(trim($l) != '') {
								$c++;
							}
						}
					}
				} else if($file[0] != '.') {
					$c += countline($dir . $file . '/');
				}
			}
			closedir($dh);
		}
	}
	
	return $c;
}

print "count of lines: " . countline($dir) . "\n";

?>
