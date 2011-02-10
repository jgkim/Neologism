<div id="node-<?php print $nid; ?>" class="term-node node<?php if ($teaser) print ' teaser'; ?>">
  <?php if ($teaser || !$page) { ?>
    <h3 id="<?php print $title; ?>"><?php print $is_class ? t('Class') : t('Property'); ?>: <?php print $qname; ?></h3>
  <?php } ?>
  <p class="summary">
    <span class="term-label"><em><?php print $term_label; ?></em></span>
    <?php if (!empty($term_comment)) { ?>
      – <span class="term-comment"><?php print $term_comment; ?></span>
    <?php } ?>
  </p>
  <dl class="term-details">
    <?php if ($has_custom_namespace) { ?>
      <dt>URI:</dt>
      <dd><?php print l($term_uri, $term_uri); ?></dd>
    <?php } ?>
    <?php if ($types) { ?>
      <dt><?php print count($types) == 1 ? t('Type') : t('Types'); ?>:</dt>
      <dd><?php print join(', ', $types); ?></dd>
    <?php } ?>
    <?php if ($domains) { ?>
      <dt><?php print count($domains) == 1 ? t('Domain') : t('Domains'); ?>:</dt>
      <dd><?php print join(', ', $domains); ?></dd>
    <?php } ?>
    <?php if ($ranges) { ?>
      <dt><?php print count($ranges) == 1 ? t('Range') : t('Ranges'); ?>:</dt>
      <dd><?php print join(', ', $ranges); ?></dd>
    <?php } ?>
    <?php if ($superproperties) { ?>
      <dt><?php print count($superproperties) == 1 ? t('Superproperty') : t('Superproperties'); ?>:</dt>
      <dd><?php print join(', ', $superproperties); ?></dd>
    <?php } ?>
    <?php if ($inverses) { ?>
      <dt><?php print count($inverses) == 1 ? t('Inverse') : t('Inverses'); ?>:</dt>
      <dd><?php print join(', ', $inverses); ?></dd>
    <?php } ?>
    <?php if ($superclasses) { ?>
      <dt><?php print count($superclasses) == 1 ? t('Superclass') : t('Superclasses'); ?>:</dt>
      <dd><?php print join(', ', $superclasses); ?></dd>
    <?php } ?>
    <?php if ($disjoints) { ?>
      <dt><?php print t('Disjoint with'); ?>:</dt>
      <dd><?php print join(', ', $disjoints); ?></dd>
    <?php } ?>
  </dl>
  <div class="description"><?php print $original_body; ?></div>
  <div class="clear-block neologism_terms_links">
    <?php if ($teaser) { ?>
      <div class="linkontop">[<a href="#sec_glance">back to top</a>]</div>
    <?php } ?>
    <div class="meta">
      <?php if ($taxonomy): ?>
        <div class="terms"><?php print $terms ?></div>
      <?php endif;?>
      <?php if ($links): ?>
        <div class="links"><?php print $links; ?></div>
      <?php endif; ?>
    </div>
  </div>
</div>
