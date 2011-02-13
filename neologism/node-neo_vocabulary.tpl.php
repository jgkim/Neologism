<div id="node-<?php print $node->nid; ?>" class="vocabulary-node node">

<?php print $picture ?>

<?php if ($page == 0): ?>
  <h2><?php print l($node->title, $node->path); ?></h2>
<?php endif; ?>

<?php if ($submitted && !$teaser): ?>
  <span class="submitted"><?php print $submitted; ?></span>
<?php endif; ?>

<div class="content clear-block">
  <?php if (count($author_list)): ?>
    <div class="authors">
      <h3>Author<?php if (count($author_list) > 1) print 's'; ?>:</h3>
      <?php
        $count = 0;
        foreach ( $author_list as $author ) {
          print '<span class="value">'.($count++ > 0 ? ', ' : '').$author.'</span>';
        }
      ?>
    </div>
  <?php endif; ?>
  <?php if( !empty($node->abstract) ): ?>
    <div class="abstract">
      <?php print $node->abstract; ?>
    </div>
  <?php endif; ?>
    
  <div class="namespace-uri">
    <h3>Namespace URI:</h3>
    <span class="value"><a href="<?php print( $node->namespace_uri ); ?>"><?php print( $node->namespace_uri ); ?></a></span>
  </div>
    
  <div class="terms-overview">
    <h3>Terms:</h3>
    <?php if ($count_classes || $count_properties) { ?>
      <span class="value">
        <?php if ($count_classes): ?>
          <?php print( ($count_classes).' '.(($count_classes == 1) ? 'Class' : 'Classes')); if ($count_properties) print ','; ?>
        <?php endif; ?>
        <?php if ($count_properties): ?>
          <?php print( ($count_properties).' '.(($count_properties == 1) ? 'Property' : 'Properties')); ?>
        <?php endif; ?>
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
