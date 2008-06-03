<?php phptemplate_comment_wrapper(NULL, $node->type); ?>

<div id="node-<?php print $node->nid; ?>" class="node<?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?>">

<?php print $picture ?>

<?php if ($page == 0): ?>
  <h2><a href="../<?php print $page ?>#<?php print $title ?>" title="<?php print $label ?>" id="<?php print $title ?>">Property <?php print $page; print ':'; print $title; ?></a></h2>
<?php endif; ?>

  <?php if ($submitted && !$teaser): ?>
    <span class="submitted"><?php print t('!date — !username', array('!username' => theme('username', $node), '!date' => format_date($node->created))); ?></span>
  <?php endif; ?>

  <div class="content">
    <?php print $content;
    
    //var_dump($content); 
    ?>
  </div>

  <div class="clear-block clear neologism_terms_links">
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
