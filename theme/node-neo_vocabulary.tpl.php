<?php

?>
<div id="node-<?php print $node->nid; ?>" class="vocabulary-node node">

<?php print $picture ?>

<?php if ($page == 0): ?>
  <h2><a href="<?php print $title ?>" title="<?php print $node->field_title[0]['value']; ?> vocabulary"><?php print $node->field_title[0]['value']; ?></a></h2>
<?php endif; ?>

  <?php if ($submitted && !$teaser): ?>
    <span class="submitted"><?php print $submitted; ?></span>
  <?php endif; ?>

    <div class="content clear-block">
    <?php if( isset($node->authors) ) { ?>
	  <div class="authors">
        <h3><?php print ((count($node->authors) > 1) ? 'Authors' : 'Author'); ?>:</h3>
<?php
        $authors = 0;
      	foreach ( $node->authors as $author ) {
          print '<span class="value">'.($authors++ > 0 ? ', ' : '').$author.'</span>';
        }
?>
      </div>
    <?php } ?>
    <?php if( !empty($node->field_abstract[0]['value']) ) { ?>
      <div class="abstract">
        <?php print $node->field_abstract[0]['value']; ?>
      </div>
    <?php } ?>
    <!--<?php print $content ?>-->
  </div>

  <div class="clear-block neologism_terms_links">
    <div class="meta">
    <?php if ($taxonomy): ?>
      <div class="terms"><?php print $terms ?></div>
    <?php endif;?>
    </div>
<?php /* TODO Commented to suppress the “Read more” link on some vocabularies. Should be done by not generating the link in the first place.
    <?php if ($links): ?>
      <div class="links"><?php print $links; ?></div>
    <?php endif; ?>
*/ ?>
  </div>

</div>
