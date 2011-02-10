<div id="node-<?php print $node->nid; ?>" class="vocabulary-node node">

<?php print $picture ?>

<?php if ($page == 0): ?>
  <h2><?php print l($node->field_title[0]['value'], $node->title); ?></h2>
<?php endif; ?>

<?php if ($submitted && !$teaser): ?>
  <span class="submitted"><?php print $submitted; ?></span>
<?php endif; ?>

<div class="content clear-block">
  <?php if( isset($node->authors) && !empty($node->authors[0]) ): ?>
    <div class="authors">
      <h3>Author<?php if (count($node->authors) > 1) print 's'; ?>:</h3>
      <?php
        $authors = 0;
        foreach ( $node->authors as $author ) {
          print '<span class="value">'.($authors++ > 0 ? ', ' : '').$author.'</span>';
        }
      ?>
    </div>
  <?php endif; ?>
  <?php if( !empty($node->field_abstract[0]['value']) ): ?>
    <div class="abstract">
      <?php print $node->field_abstract[0]['value']; ?>
    </div>
  <?php endif; ?>
    
  <div class="namespace-uri">
    <h3>Namespace URI:</h3>
    <span class="value"><a href="<?php print( $node->namespace_uri ); ?>"><?php print( $node->namespace_uri ); ?></a></span>
  </div>
    
  <div class="terms-overview">
    <h3>Terms:</h3>
    <?php if ($node->count_classes || $node->count_properties) { ?>
      <span class="value">
        <?php print( ($node->count_classes).' '.(($node->count_classes == 1) ? 'Class' : 'Classes')); ?>,
        <?php print( ($node->count_properties).' '.(($node->count_properties == 1) ? 'Property' : 'Properties')); ?>.
      </span>
    <?php } else { ?>
      No classes or properties defined yet.
    <?php } ?>
  </div>
</div>

<div class="clear-block neologism_terms_links">
  <div class="meta">
  <?php if ($taxonomy): ?>
    <div class="terms"><?php print $terms ?></div>
  <?php endif;?>
  </div>
  <?php if ($links): ?>
    <div class="links"><?php print $links; ?></div>
  <?php endif; ?>
</div>

</div>
