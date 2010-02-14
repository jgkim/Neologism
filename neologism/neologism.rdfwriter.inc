<?php

include_once(drupal_get_path('module', 'rdf') . '/vendor/arc/ARC2.php');
ARC2::inc('TurtleSerializer');
ARC2::inc('RDFXMLSerializer');

/**
 * A writer for RDF statements. Encapsulates RAP storage and serialisation,
 * has its own namespace management, and provides a convenient API for
 * generating RDF statements.
 */
class NeologismRDFWriter {
  var $_namespaces = array();
  var $_triples = array();

  /**
   * Registers a namespace mapping that will be added to the written
   * RDF file.
   *
   * @param $prefix
   *   A namespace prefix, such as 'foaf'
   * @param $uri
   *   A namespace URI, such as 'http://xmlns.com/foaf/0.1/'
   */
  function register_namespace($prefix, $uri) {
    if (in_array($uri, $this->_namespaces)) return;
    $this->_namespaces[$prefix] = $uri;
  }

  function triple_literal($s, $p, $o) {
    $this->_triple($s, $p, $o, 'literal');
  }

  function triple_uri($s, $p, $o) {
    $this->_triple($s, $p, $o, 'uri');
  }

  function triple_qname($s, $p, $o) {
    $this->_triple($s, $p, $this->_expand_qname($o), 'uri');
  }

  function triples_qname($s, $p, $os) {
    if (empty($os)) return;
    if (!is_array($os)) {
      throw new Exception("Not an array: '$os'");
    }
    foreach ($os as $qname) {
      $this->triple_qname($s, $p, $qname);
    }
  }

  function _triple($s, $p, $o, $o_type) {
    if (empty($s) || empty($p) || empty($o)) return;
    $p = $this->_expand_qname($p);
    if (!$p) return;
    $this->_triples[] = array('s' => $s, 's_type' => 'uri', 'p' => $p, 'o' => $o, 'o_type' => $o_type);
  }

  function _expand_qname($qname) {
    list($prefix, $local) = explode(':', $qname);
    if (!isset($this->_namespaces[$prefix])) return null;
    return $this->_namespaces[$prefix] . $local;
  }

  function write_rdfxml() {
    drupal_set_header("Content-Type: application/rdf+xml; charset=utf-8");
    module_load_include('inc', 'neologism', 'neologism.arc');
    $ser = ARC2::getRDFXMLSerializer(array('ns' => $this->_namespaces, 'serializer_type_nodes' => true));
    echo $ser->getSerializedTriples($this->_triples);
  }

  function write_n3() {
    drupal_set_header("Content-Type: text/rdf+n3; charset=utf-8");
    module_load_include('inc', 'neologism', 'neologism.arc');
    $ser = new NeologismTurtleSerializer($this->_namespaces);
    echo $ser->getSerializedTriples($this->_triples);
  }
}

/**
 * A customized Turtle serializer, based on the one inculded in ARC2.
 * It uses slightly different rules for compressing URIs into QNames,
 * to make more pleasing output. We keep it in a separate include file
 * to avoid having to include ARC2.php all the time.
 */
class NeologismTurtleSerializer extends ARC2_TurtleSerializer {

  function __construct($namespaces) {
    parent::__construct(array('ns' => $namespaces), new stdClass());
  }

  function getTerm($v, $term = '', $qualifier = '') {
    if ($v === 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type' && $term === 'p') return 'a';
    if (!is_array($v) && ($term === 's' || $term === 'o')) {
      foreach ($this->ns as $prefix => $uri) {
        if (strpos($v, $uri) === 0) {
          $local = substr($v, strlen($uri));
          if (preg_match('/^[a-z0-9_][a-z0-9_-]*$/i', $local)) {
            if (!in_array($uri, $this->used_ns)) $this->used_ns[] = $uri;
            return $prefix . ':' . $local;
          }
        }
      }
    }
    return parent::getTerm($v, $term, $qualifier);
  }
  
  function getSerializedIndex($index, $raw = 0) {
    $r = '';
    $nl = "\n";
    foreach ($index as $s => $ps) {
      $r .= $r ? ' .' . $nl . $nl : '';
      $s = $this->getTerm($s, 's');
      $r .= $s;
      $first_p = 1;
      foreach ($ps as $p => $os) {
        if (!$os) continue;
        $p = $this->getTerm($p, 'p');
        $r .= $p === 'a' ? ' ' : ($first_p ? '' : ';') . $nl . '    ';
        $r .= $p;
        $first_o = 1;
        if (!is_array($os)) {/* single literal o */
          $os = array(array('value' => $os, 'type' => 'literal'));
        }
        if (count($os) == 1) {
          $r .= ' ' . $this->getTerm($os[0], 'o', $p);
        } else {
          foreach ($os as $o) {
            $r .= $p === 'a' ? ($first_o ? ' ' : ', ') : ($first_o ? '' : ',') . $nl . '        ';
            $o = $this->getTerm($o, 'o', $p);
            $r .= $o;
            $first_o = 0;
          }
        }
        $first_p = 0;
      }
    }
    $r .= $r ? ' .' . $nl : '';
    if ($raw) {
      return $r;
    }
    return $r ? $this->getHead() . $nl . $nl . $r : '';
  }
}
