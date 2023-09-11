<?php

/**
 * Generator to generate enum converter functions
 *
 * This script is written in PHP because it was easy.
 * If there is a better / simpler solution let us know,
 * over Pull Request or issues
 */

$dh = opendir('.');

while (($file = readdir($dh)) !== false) {
	if ($file[0] == '.') {
		continue;
	}

	if ($file != 'Action.enum.h') {
		continue;
	}

	generateEnum($file);
	echo "== $file ==\n";
}

closedir($dh);

function parseEnumFile($file) {
	$inEnum = false;
	$values = array();
	foreach (file($file) as $line) {
		$line = trim($line);

		if (!$inEnum && substr($line, 0, 5) == 'enum ') {
			$inEnum = true;
			continue;
		}
		if (!$inEnum) {
			continue;
		}
		if (substr($line, 0, 1) == '{') {
			continue;
		}
		if (substr($line, 0, 2) == '};') {
			$inEnum = false;
			continue;
		}
		if ($line == '' || substr($line, 0, 2) == '//') {
			continue;
		}

		$pos = strrpos($line, "_COUNT");
        if ($pos !== false) {
            continue;
        }

		$pos = strrpos($line, '=');
		if ($pos !== false) {
            print "ERROR: non contiguous enum!!\n";
			$line = substr($line, 0, $pos);
		}
		$pos = strrpos($line, ',');
		if ($pos !== false) {
			$line = substr($line, 0, $pos);
		}

		$line = trim($line);
		$values[] = $line;
	}

	return $values;
}

function writeCppFile($output, $name, $values) {
    $fp = fopen("$output", 'w');
    $tab = "    "; // 4 tab space as specified in .clang_format

    fwrite($fp, "// ** THIS FILE IS GENERATED **\n");
    fwrite($fp, "// ** use generateConvertNEW.php to update this file **\n");
    fwrite($fp, "\n\n");

    fwrite($fp, "#pragma once\n\n");

    fwrite($fp, "constexpr const char* ACTION_NAMES[] = {  // $name to string conversion map");

    foreach ($values as $v) {
        $v = str_replace("_","-",$v);
        fwrite($fp, "\n" . $tab . $tab . "\"". strtolower($v) . "\",");
    }
    fseek($fp,-1,SEEK_CUR);
    fwrite($fp, "};\n");

    fclose($fp);

	print "Generated $output\n";
}

function generateEnum($file) {
	$values = parseEnumFile($file);

	$name = substr($file, 0, -7);
	writeCppFile("generated/${name}.NameMap.generated.h", $name, $values);
}

