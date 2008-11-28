<?php

/********************************************************************************
*										*
*	Version: content_negotiation.inc.php v1.2.0 2007-12-05			*
*	Copyright: (c) 2006-2007 ptlis						*
*	Licence: GNU Lesser General Public License v2.1				*
*	The current version of this library can be sourced from:		*
*		http://ptlis.net/source/php-content-negotiation/		*
*	Contact the author of this library at:					*
*		ptlis@ptlis.net							*
*										*
*	This class requires PHP 5.x (it has been tested on 5.0.x - 5.2		*
*	with error reporting set to E_ALL | E_STRICT without problems)		*
*	but should be trivially modifiable to work on PHP 4.x			*
*										*
********************************************************************************/

define('WILDCARD_DEFAULT',  -1);
define('WILDCARD_TYPE', 0);
define('WILDCARD_SUBTYPE', 1);
define('WILDCARD_NONE', 2);

class content_negotiation {
	// generic private function called by all other public ones
	static private function generic_negotiation($header, $return_type, $content_match, $mime_negotiation=false) {
		if(is_array($content_match) && count($content_match) > 0) {
			$app_vals_provided	= true;
			$content_match_count	= count($content_match['type']);
			for($i = 0; $i < $content_match_count; $i++) {			// Set default values (and make all lower case)
				$content_match['type'][$i] = strtolower($content_match['type'][$i]);
				$content_match['q_value'][$i] = 0;
				$content_match['specificness'][$i] = WILDCARD_DEFAULT;
			}
		}
		else {
			$app_vals_provided			= false;
			$content_match['type']			= array();
			$content_match['q_value']		= array();
		}

		$accept_list = explode(",", strtolower(str_replace(' ', null, $header)));
		$accept_list_count = count($accept_list);

		foreach($accept_list as $accept_value) {
			$accept_value_parts = explode(";", $accept_value);
			if($mime_negotiation) {					// Special case for wildcard rules
				$type_array = explode('/', $accept_value_parts[0]);
			}

			if($app_vals_provided && (($mime_negotiation && $type_array[0] === '*' && $type_array[1] === '*') || (!$mime_negotiation && $accept_value_parts[0] === '*'))) {	// App values are provided, and there's a wildcard for type & subtype (wildcards only used if app values are provided)
				for($array_position = 0; $array_position < $content_match_count; $array_position++) {
					if($content_match['specificness'][$array_position] === WILDCARD_DEFAULT) {	// Only store new value if the current value is defualt (type wildcard has least precidence)
						$content_match['specificness'][$array_position] = WILDCARD_TYPE;
						if(isset($accept_value_parts[1])) {
							$content_match['q_value'][$array_position]	= content_negotiation::parse_q($accept_value_parts[1], $content_match);
						}
						else {
							$content_match['q_value'][$array_position]	= 1;
						}
					}
				}
			}
			else if($app_vals_provided && $mime_negotiation && $type_array[1] === '*') {											// App values are provided, and there's a wildcard for subtype (wildcards only used if app values are provided)
				for($array_position = 0; $array_position < $content_match_count; $array_position++) {
					if(preg_match('/^' . $type_array[0] . '\/[a-zA-z\+\-]/', $content_match['type'][$array_position]) && $content_match['specificn=ess'][$array_position] <= WILDCARD_TYPE) {
						$content_match['specificness'][$array_position] = WILDCARD_SUBTYPE;
						if(isset($accept_value_parts[1])) {
							$content_match['q_value'][$array_position]	= content_negotiation::parse_q($accept_value_parts[1], $content_match);
						}
						else {
							$content_match['q_value'][$array_position]	= 1;
						}
					}
				}
			}
			else if($app_vals_provided) {																	// App values are provided, but this type is not a wildcard
				$array_position = array_search($accept_value_parts[0], $content_match['type']);
				if(is_numeric($array_position)) {
					$content_match['specificness'][$array_position] = WILDCARD_NONE;
					if(isset($accept_value_parts[1])) {
						$content_match['q_value'][$array_position]	= content_negotiation::parse_q($accept_value_parts[1], $content_match);
					}
					else {
						$content_match['q_value'][$array_position]	= 1;
					}
				}
			}
			else {																				// No app values are provided, ignore wildcards
				if(($mime_negotiation && $type_array[0] !== '*' && $type_array[1] !== '*') || (!$mime_negotiation && $accept_value_parts[0] !== '*')) {
					array_push($content_match['type'], $accept_value_parts[0]);
					if(isset($accept_value_parts[1])) {
						array_push($content_match['q_value'], content_negotiation::parse_q($accept_value_parts[1], $content_match));
					}
					else {
						array_push($content_match['q_value'], 1);
					}
				}
			}
		}

		if($app_vals_provided) {
			array_multisort($content_match['q_value'], SORT_DESC, SORT_NUMERIC,
					$content_match['app_preference'], SORT_DESC, SORT_STRING,
					$content_match['type'],
					$content_match['specificness']);
		}
		else {
			array_multisort($content_match['q_value'], SORT_DESC, SORT_NUMERIC,
					$content_match['type']);
		}

		switch($return_type) {
			case 'all':
				return $content_match;
				break;
			case 'best':
				return $content_match['type'][0];
				break;
			default:
				return false;
				break;
		}
	}


	// Parses q value from string
	static private function parse_q($q_string, &$content_match) {
		if(preg_match('/q=(0\.\d{1,5}|1\.0|[01])/i', $q_string, $matches)) {
			return $matches[1]; 
		} else {	// On unparsable q value default to 1
			return 1;
		}
	}


	// return only the preferred mime-type
	static public function mime_best_negotiation($content_match=null) {
		if(isset($_SERVER['HTTP_ACCEPT'])) {
			$header = $_SERVER['HTTP_ACCEPT'];
		}
		else {
			return false;
		}

		return content_negotiation::generic_negotiation($header, 'best', $content_match, true);
	}


	// return the whole array of mime-types
	static public function mime_all_negotiation($content_match=null) {
		if(isset($_SERVER['HTTP_ACCEPT'])) {
			$header = $_SERVER['HTTP_ACCEPT'];
		}
		else {
			return false;
		}

		return content_negotiation::generic_negotiation($header, 'all', $content_match, true);
	}


	// return only the preferred charset
	static public function charset_best_negotiation($content_match=null) {
		if(isset($_SERVER['HTTP_ACCEPT_CHARSET'])) {
			$header = $_SERVER['HTTP_ACCEPT_CHARSET'];
		}
		else {
			return false;
		}

		return content_negotiation::generic_negotiation($header, 'best', $content_match);
	}


	// return the whole array of charsets
	static public function charset_all_negotiation($content_match=null) {
		if(isset($_SERVER['HTTP_ACCEPT_CHARSET'])) {
			$header = $_SERVER['HTTP_ACCEPT_CHARSET'];
		}
		else {
			return false;
		}

		return content_negotiation::generic_negotiation($header, 'all', $content_match);
	}


	// return only the preferred encoding-type
	static public function encoding_best_negotiation($content_match=null) {
		if(isset($_SERVER['HTTP_ACCEPT_ENCODING'])) {
			$header = $_SERVER['HTTP_ACCEPT_ENCODING'];
		}
		else {
			return false;
		}

		return content_negotiation::generic_negotiation($header, 'best', $content_match);
	}


	// return the whole array of encoding-types
	static public function encoding_all_negotiation($content_match=null) {
		if(isset($_SERVER['HTTP_ACCEPT_ENCODING'])) {
			$header = $_SERVER['HTTP_ACCEPT_ENCODING'];
		}
		else {
			return false;
		}

		return content_negotiation::generic_negotiation($header, 'all', $content_match);
	}


	// return only the preferred language
	static public function language_best_negotiation($content_match=null) {
		if(isset($_SERVER['HTTP_ACCEPT_LANGUAGE'])) {
			$header = $_SERVER['HTTP_ACCEPT_LANGUAGE'];
		}
		else {
			return false;
		}

		return content_negotiation::generic_negotiation($header, 'best', $content_match);
	}


	// return the whole array of language
	static public function language_all_negotiation($content_match=null) {
		if(isset($_SERVER['HTTP_ACCEPT_LANGUAGE'])) {
			$header = $_SERVER['HTTP_ACCEPT_LANGUAGE'];
		}
		else {
			return false;
		}

		return content_negotiation::generic_negotiation($header, 'all', $content_match);
	}
}

?>
