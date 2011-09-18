<?php

/**
 * Checks the configuration, if you have installed the required packages etc.
 * @author Andreas Butti
 *
 */

class Configuration {
	/**
	 * Contents of config.ini
	 */
	private $config;

	/**
	 * The libs.xml file
	 */
	private $xml = null;

	/**
	 * Save the checked options
	 */
	private $options = array();

	public function __construct() {
		$this->config = parse_ini_file(dirname(__FILE__) . '/../config.ini');
	}

	private function parseXml() {
		if($this->xml == null) {
			$file = dirname(__FILE__) . '/../libs.xml';
			$this->xml = simplexml_load_file($file);
		}
	}

	public function getConfig() {
		return $this->config;
	}
	
	private function checkLib($name, $version) {
		print "checking for library {$name} >= {$version}...\n";

		$returnVal = -1;
		passthru("{$this->config['PKG_CONFIG']} --exists --print-errors \"{$name} >= $version\"", $returnVal);

		if($returnVal == 0) {
			print "Found.\n\n";
			return true;
		} else {
			print "Not found.\n\n";
			return false;
		}
	}

	public function checkLibraries() {
		$this->parseXml();

		$ok = true;

		foreach($this->xml->required as $required) {
			foreach($required as $lib) {
				$version = (string)$lib->attributes()->version;
				$name = (string)$lib;
					
				if(!$this->checkLib($name, $version)) {
					$ok = false;
				}
			}
		}


		foreach($this->xml->option as $option) {
			$oname = (string)$option->attributes()->name;
				
			$optionOk = true;
			foreach($option as $lib) {
				$version = (string)$lib->attributes()->version;
				$name = (string)$lib;
					
				if(!$this->checkLib($name, $version)) {
					$optionOk = false;
				}
			}
				
			$this->options[$oname] = $optionOk;
		}

		foreach($this->options as $name => $enabled) {
			print "All requirements for option {$name} are " . ($enabled ? 'available' : 'not available') . "\n";
		}

		return $ok;
	}

	public function getLibs() {
		$libs = array();
		
		foreach($this->xml->required as $required) {
			foreach($required as $lib) {
				$name = (string)$lib;
				$libs[] = $name;
			}
		}
		
		return $libs;
	}
}

