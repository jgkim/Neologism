<?php
// $Id: node.tpl.php,v 1.5 2007/10/11 09:51:29 goba Exp $
?>
<div id="node-<?php print $node->nid; ?>" class="node<?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?>">

<?php print $picture ?>

<?php if ($page == 0): ?>
  <h2><a href="../<?php print $page ?>#<?php print $title ?>" title="<?php print $label ?>" id="<?php print $title ?>">Property <?php print $page; print ':'; print $title; ?></a></h2>
<?php endif; ?>

  <?php if ($submitted && !$teaser): ?>
    <span class="submitted"><?php print $submitted; ?></span>
  <?php endif; ?>

  <div class="content clear-block">
    <?php print $content ?>
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
