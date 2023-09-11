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

		$pos = strrpos($line, "_MAX_VALUE");
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

    fwrite($fp, "#include <string_view>  // for string_view\n\n");
    fwrite($fp, "#include <glib.h>  // for g_warning\n\n");
    fwrite($fp, "#include \"../$name.enum.h\"\n");
    fwrite($fp, "#include \"util/Assert.h\"  // for xoj_assert\n\n");


    fwrite($fp, "// ** This needs to be copied to the header\n");
    fwrite($fp, "" . $name . " " . $name . "_fromString(const std::string_view value);\n");
    fwrite($fp, "const char* " . $name . "_toString($name value);\n");

    fwrite($fp, "\n\n");

    fwrite($fp, "constexpr const char* NAMES[] = {  // $name to string conversion map");

    foreach ($values as $v) {
        $v = str_replace("_","-",$v);
        fwrite($fp, "\n" . $tab . $tab . "\"". strtolower($v) . "\",");
    }
    fseek($fp,-1,SEEK_CUR);
    fwrite($fp, "};");

    ////////////////////////////////////////////////////////////////////////////

    fwrite($fp, "\n\n");

    fwrite($fp, "auto " . $name . "_toString($name value) -> const char* {\n");
    fwrite($fp, $tab . "xoj_assert(value <= $name::_MAX_VALUE);\n");
    fwrite($fp, $tab . "return NAMES[static_cast<size_t>(value)];\n");
    fwrite($fp, "}\n\n");

    fwrite($fp, "auto " . $name . "_fromString(const std::string_view value) -> " . $name . " {\n");
    fwrite($fp, $tab . "for (size_t n = 0; n <= static_cast<size_t>(" . $name . "::_MAX_VALUE); n++) {\n");
    fwrite($fp, $tab . $tab . "if (value == NAMES[n]) {\n");
    fwrite($fp, $tab . $tab . $tab . "return static_cast<" . $name . ">(n);\n");
    fwrite($fp, $tab . $tab . "}\n");
    fwrite($fp, $tab . "}\n");

    fwrite($fp, $tab . "g_warning(\"Invalid enum value for $name: \\\"%s\\\"\", value.data());\n");
    fwrite($fp, $tab . "return $name::{$values[0]};\n");


    fwrite($fp, "}\n");


    fclose($fp);

	print "Generated $output\n";
}

function generateEnum($file) {
	$values = parseEnumFile($file);

	$name = substr($file, 0, -7);
	writeCppFile("generated/$name.generated.new.cpp", $name, $values);
}

