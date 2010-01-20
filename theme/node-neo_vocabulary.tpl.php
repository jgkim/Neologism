<?php

?>
<div id="node-<?php print $node->nid; ?>" class="vocabulary-node">

<?php print $picture ?>

<?php if ($page == 0): ?>
  <h2><a href="<?php print $title ?>" title="<?php print $node->field_title[0]['value']; ?> vocabulary"><?php print $node->field_title[0]['value']; ?></a></h2>
<?php endif; ?>

  <?php if ($submitted && !$teaser): ?>
    <span class="submitted"><?php print $submitted; ?></span>
  <?php endif; ?>

  <div class="content clear-block">
	<div class="authors" ><h3>Author(s):</h3>
    <?php
      if( isset($node->authors) ) {
        $authors = 0;
      	foreach ( $node->authors as $author ) {
          print '<span class="value">'.($authors++ > 0 ? ', ' : '').$author.'</span>';
        }
      }
    ?>
    </div>
    <?php if( !empty($node->field_abstract[0]['value']) ) { ?>
      <div class="abstract">
        <h3>Abstract</h3>
        <?php print $node->field_abstract[0]['value']; ?>
      </div>
    <?php } ?>
    <!--<?php print $content ?>-->
  </div>

  <div class="clear-block neologism_terms_links">
 		<?php if ($teaser): ?>
			<div class="linkontop">[<a href="#sec_glance">back to top</a>]</div>
		<?php endif; ?>

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
