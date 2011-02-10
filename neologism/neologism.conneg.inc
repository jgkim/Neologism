<?php
// $Id$

/**
 * This file has code for content negotiation, and enables the
 * caching of content-negotiated responses in Drupal.
 *
 * The evaluation of the Accept HTTP header against the list of
 * supported media types is performed by an external library.
 *
 * When page caching is enabled in Drupal, then cached pages are
 * returned before modules have any chance of intercepting the
 * response. Hence it is impossible for a module to give different
 * responses based on HTTP headers when the page is already cached.
 *
 * We handle this problem by disabling the normal page caching for
 * content-negotiated pages. No content will ever be stored in the
 * page cache for content-negotiated URIs.
 *
 * To nevertheless get the benefits of caching, we code our own
 * cache handling that is invoked via hook_init() and hook_exit().
 *
 * The idea is to store responses in the cache not under the
 * content-negotiated URI, but under the URI of the specific
 * variant that was generated. Then we can check in the init
 * phase wether a cache entry already exists for the desired
 * variant.
 *
 * For example, if the page at path "generic" is loaded, and the
 * Accept header indicates a preference for HTML, then we look in
 * the cache for an entry for "generic.html", and respond it if
 * it exists. If it doesn't exist, then the page is generated as
 * usual, and finally stored under "generic.html".
 *
 * A direct access to "generic.html" would also access the same
 * cache record.
 *
 * Achieving this caching behavior requires some messing with the
 * internals of Drupal, for example, temporarily modifying the
 * value of $_SERVER['REQUEST_URI'].
 */

/**
 * Invokes one of several handlers, depending on the variant
 * chosen by the content negotiation logic.
 *
 * @param $node
 *   A node to be passed to the handler
 * @param $variant_handlers
 *   Associative array with variant names as keys and function
 *   names as values
 * @return
 *   The result of invoking the handler appropriate for the
 *   best variant
 */
function _neologism_do_conneg(&$node, $variant_handlers) {
  _neologism_set_is_negotiated();
  global $neologism_variant;
  if (!isset($variant_handlers[$neologism_variant])) {
    drupal_set_message(t("Internal error: Preferred variant '%variant' has no handler", array('%variant' => $neologism_variant)), 'error');
    return;
  }
  return $variant_handlers[$neologism_variant]($node);
}

/**
 * Computes the variant name ('rdf', 'html', etc.) that best
 * matches the Accept HTTP header sent by the client.
 *
 * @return
 *   A variant name such as 'html', 'rdf'
 */
function _neologism_get_preferred_variant() {
  include_once(drupal_get_path('module', 'neologism') . 
      '/includes/content_negotiation/content_negotiation.inc.php');

  // Stupid browsers that send */* should get HTML, so give
  // highest preference ot that. We prefer XHTML over HTML if
  // both are supported. Among RDF formats, RDF/XML has precedence
  // over Turtle and N3. We also map application/xml to
  // RDF/XML.
  $supported_types = array(
    array('html', 1.00, 'application/xhtml+xml'),
    array('html', 0.99, 'text/html'),
    array('rdf',  0.90, 'application/rdf+xml'),
    array('rdf',  0.50, 'application/xml'),
    array('ttl',  0.88, 'text/rdf+n3'),
    array('ttl',  0.89, 'text/turtle'),
    array('ttl',  0.87, 'text/x-turtle'),
    array('ttl',  0.86, 'application/turtle'),
    array('ttl',  0.85, 'application/x-turtle'),
  );
  $default = 'rdf';

  $struct = array('type' => array(), 'app_preference' => array());
  foreach ($supported_types as $type) {
    $struct['type'][] = $type[2];
    $struct['app_preference'][] = $type[1];
  }
  $best = content_negotiation::mime_best_negotiation($struct);
  foreach ($supported_types as $type) {
    if ($type[2] == $best) {
      return $type[0];
    }
  }
  return $default;
}

/**
 * Returns the URI of the current page, with a variant file extension
 * appended, such as 'rdf' or 'html'.
 *
 * @param $variant
 *   A variant name, such as 'rdf' or 'html'
 * @param $absoulte
 *   If TRUE, an absolute URI will be returned. If false, the REQUEST_URI for the variant will be returned.
 * @return
 *   The URI of the specified variant
 */
function _neologism_get_variant_uri($variant, $absolute = TRUE) {
  // We use $_REQUEST instead of $_GET because we want the original
  // URI, without alias rewriting
  $path = $absolute ? (isset($_REQUEST['q']) ? $_REQUEST['q'] : '<front>') : request_uri();
  if (substr($path, -1) == '/') {
    $path .= $variant;
  } else {
    $path .= '.' . $variant;
  }
  return $absolute ? url($path, array('absolute' => TRUE)) : $path;
}

/**
 * Checks the Accept HTTP header to determine the best
 * variant; checks wether a page is cached for that variant's
 * URI; returns it and dies if a cached page was found.
 * This is typically called from hook_init().
 */
function _neologism_negotiated_cache_start() {
  global $neologism_variant, $neologism_is_negotiated;
  $neologism_is_negotiated = FALSE;
  $neologism_variant = _neologism_get_preferred_variant();

  $cache_mode = variable_get('cache', CACHE_DISABLED);
  $cache = '';
  if ($cache_mode != CACHE_DISABLED) {
    // An output buffer for the cache has probably already been
    // set up during bootstrap; close it, because we will start
    // a new one
    if (page_get_cache(TRUE)) {
      ob_end_clean();
    }
    // temporarily change REQUEST_URI to the variant's URI; this
    // will cause page_get_cache to check the cache for that URI
    $temp = $_SERVER['REQUEST_URI'];
    $_SERVER['REQUEST_URI'] = _neologism_get_variant_uri($neologism_variant, FALSE);
    $cache = page_get_cache();
    $_SERVER['REQUEST_URI'] = $temp;
  }

  // Not yet cached? Just return and process as usual
  if (!$cache) return;

  // It's a negotiated response
  _neologism_set_negotiation_headers();
  // Output the cached body and headers
  drupal_page_cache_header($cache);

  // Try to properly shut down Drupal before exiting. This is not quite
  // complete because other modules may have been already initialized.
  bootstrap_invoke_all('exit');
  // We are done, stop processing.
  exit;
}

/**
 * Marks this page as being subject to negotiation. This will
 * switch off the standard page cache.
 */
function _neologism_set_is_negotiated() {
  global $neologism_cache_setting, $neologism_is_negotiated;
  $neologism_is_negotiated = TRUE;
  // store cache settings so we can restore them later
  $neologism_cache_setting = $GLOBALS['conf']['cache'];
  // caching off
  $GLOBALS['conf']['cache'] = FALSE;
  _neologism_set_negotiation_headers();
}

/**
 * Checks if the current page has been marked as negotiated, and if it is,
 * then it will be stored in the cache under the variant's URI.
 * This is typically called from hook_exit().
 */
function _neologism_negotiated_cache_end() {
  global $neologism_is_negotiated, $neologism_cache_setting, $neologism_variant;
  if (!$neologism_is_negotiated) return;
  // restore original cache settings
  $GLOBALS['conf']['cache'] = $neologism_cache_setting;

  // temporarily change REQUEST_URI to trick page_set_cache into storing
  // the page at the variant URI
  $temp = $_SERVER['REQUEST_URI'];
  $_SERVER['REQUEST_URI'] = _neologism_get_variant_uri($neologism_variant, FALSE);
  if (variable_get('cache', CACHE_DISABLED) != CACHE_DISABLED) {
    // store page in cache
    page_set_cache();
  }
  $_SERVER['REQUEST_URI'] = $temp;
}

/**
 * Sets the Vary and Content-Location HTTP headers. Must be invoked for
 * any response that is subject to negotiation, including cached responses.
 */
function _neologism_set_negotiation_headers() {
  global $neologism_variant;
  drupal_set_header("Vary: Accept");
  drupal_set_header("Content-Location: " . _neologism_get_variant_uri($neologism_variant, TRUE));
}

