<?php

/**
 * Parses the source.xml file
 * @author Andreas Butti
 */

class SourceParser {
	/**
	 *
	 * Array: [Group] array of files
	 */
	private $data = array();

	/**
	 * Our listed files
	 */
	private $list = array();

	public function __construct() {
	}

	public function load($file = null) {
		if($file == null) {
			$file = dirname(__FILE__) . '/../sources.xml';
		}
		$xml = simplexml_load_file($file);

		$this->data = array();

		foreach($xml->group as $g) {
			$name = (string)$g->attributes()->name;
			if(!isset($this->data[$name])) {
				$this->data[$name] = array();
			}

			foreach($g->file as $f) {
				$this->data[$name][] = (string)$f;
			}
		}
	}

	public function readFolder($dir, $subdir) {
		if ($dh = opendir($dir)) {
			while (($file = readdir($dh)) !== false) {
				if($file[0] == '.') {
					continue;
				}
				if (is_dir($dir . $file)) {
					$this->readFolder($dir . $file . '/', $subdir . $file . '/');
				} else if(substr($file, -4) == '.cpp' || substr($file, -2) == '.c') {
					$this->list[] = $subdir . $file;
				}
			}
			closedir($dh);
		}
	}

	public function checkFiles($dir) {
		$this->list = array();
		$this->readFolder($dir, '');

		foreach($this->data as $gname => $group) {
			foreach($group as $id => $file) {
				if(!in_array($file, $this->list) && substr($file, 0, 10) != '[MISSING!]') {
					$this->data[$gname][$id] = '[MISSING!] ' . $file;
				}
			}
		}

		foreach($this->list as $d) {
			$found = false;
			foreach($this->data as $gname => $group) {
				if(in_array($d, $group)) {
					$found = true;
					break;
				}
			}
			
			if(!$found) {
				if(!isset($this->data['new files'])) {
					$this->data['new files'] = array();
				}
				$this->data['new files'][] = $d;
			}
		}
	}

	public function getData() {
		return $this->data;
	}
	
	public function save($file = null) {
		if($file == null) {
			$file = dirname(__FILE__) . '/../sources.xml';
		}
	
		$fp = fopen($file, 'w');
		
		fwrite($fp, '<?xml version="1.0" encoding="UTF-8"?>' . "\n<sources>\n");
		
		foreach($this->data as $name => $group) {
			fwrite($fp, "\t<group name=\"{$name}\">\n"); // should be encoded!

			foreach($group as $file) {
				fwrite($fp, "\t\t<file>{$file}</file>\n"); // should be encoded!
			}
			
			fwrite($fp, "\t</group>\n"); // should be encoded!
		}
		
		fwrite($fp, "</sources>\n");

		fclose($fp);
	}
	
}
