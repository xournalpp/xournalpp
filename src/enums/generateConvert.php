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

	if (substr($file, -7) != '.enum.h') {
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
		
		$pos = strrpos($line, '=');
		if ($pos !== false) {
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
	$fp = fopen($output, 'w');
    $tab = "    "; // 4 tab space as specified in .clang_format

	fwrite($fp, "// ** THIS FILE IS GENERATED **\n");
	fwrite($fp, "// ** use generateConvert.php to update this file **\n");
	fwrite($fp, "\n\n");

	fwrite($fp, "#include <string>\n\n");
	fwrite($fp, "#include \"../$name.enum.h\"\n\n");
	fwrite($fp, "using std::string;\n");
	fwrite($fp, "#include <glib.h>\n");


	fwrite($fp, "\n\n");

	fwrite($fp, "// ** This needs to be copied to the header\n");
	fwrite($fp, $name . " " . $name . "_fromString(const string& value);\n");
	fwrite($fp, "string " . $name . "_toString($name value);\n");

	fwrite($fp, "\n\n");

	fwrite($fp, "auto " . $name . "_fromString(const string& value) -> " . $name . " {\n");
	
	foreach ($values as $v) {
		fwrite($fp, $tab . "if (value == \"$v\") {\n");
		fwrite($fp, $tab . $tab . "return $v;\n");
		fwrite($fp, $tab . "}\n");
		fwrite($fp, "\n");
	}

	fwrite($fp, $tab . "g_warning(\"Invalid enum value for $name: «%s»\", value.c_str());\n");
	fwrite($fp, $tab . "return {$values[0]};\n");

	
	fwrite($fp, "}\n");

	////////////////////////////////////////////////////////////////////////////
	
	fwrite($fp, "\n\n");

	fwrite($fp, "auto " . $name . "_toString($name value) -> string {\n");
	
	foreach ($values as $v) {
		fwrite($fp, $tab . "if (value == $v) {\n");
		fwrite($fp, $tab . $tab . "return \"$v\";\n");
		fwrite($fp, $tab . "}\n");
		fwrite($fp, "\n");
	}

	fwrite($fp, $tab . "g_error(\"Invalid enum value for $name: %i\", value);\n");
	fwrite($fp, $tab . "return \"\";\n");

	
	fwrite($fp, "}\n");



	fclose($fp);
	print "Generated $output\n";
}

function generateEnum($file) {
	$values = parseEnumFile($file);
	
	$name = substr($file, 0, -7);
	writeCppFile("generated/$name.generated.cpp", $name, $values);
}

