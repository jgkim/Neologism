<?php if (!$field_empty) : ?>
<div class="field field-type-<?php print $field_type_css ?> field-<?php print $field_name_css ?>">
  <?php if ($label_display == 'above') : ?>
    <div class="field-label"><?php print $label ?>:&nbsp;</div>
  <?php endif;?>
  <div class="field-items">
    <?php foreach ($items as $delta => $item) :
            if (!empty($item['view']) || $item['view'] === "0") : ?>
              <div class="field-item">
                <?php if ($label_display == 'inline') { ?>
                  <div class="field-label-inline<?php print($delta ? '' : '-first')?>">
                    <?php print $label ?>:&nbsp;</div>
                <?php } ?>
                <?php print $item['view'] ?>
              </div>
      <?php endif;
          endforeach;?>
  </div>
</div>
<?php endif; ?>
