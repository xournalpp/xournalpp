<?php

/**
 * This scripts builds the application
 */

include dirname(__FILE__) . '/inc.php/sourceXmlParser.php';
include dirname(__FILE__) . '/inc.php/config.php';
include dirname(__FILE__) . '/inc.php/builder.php';

$config = new Configuration();

if(!$config->checkLibraries()) {
	print "Build canceled, not all required libraries are installed.\n\n";
	return 1;
}

$parser = new SourceParser();
$parser->load();


$builder = new Builder($config, $parser);
$builder->build(dirname(__FILE__) . '/../src/', dirname(__FILE__) . '/build/obj/', dirname(__FILE__) . '/build/xournalpp', 'xournalpp');

