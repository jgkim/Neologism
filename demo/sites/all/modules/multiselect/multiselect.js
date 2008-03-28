// $Id $

//@file
//Code that operates multiselect boxes
//@author Obslogic (Mike Smith aka Lionfish)

$(document).ready(function()
{
  //remove the items that haven't been selected from the select box.
  $('select.multiselect_unsel').each(function()
  {
    unselclass = '.' + this.id + '_unsel';
    selclass = '.' + this.id + '_sel';
    $(unselclass).removeContentsFrom($(selclass));   
  });

  //note: Doesn't matter what sort of submit button it is really (preview or submit)
  //selects all the items in the selected box (so they are actually selected) when submitted
  $('input.form-submit').click(function()
  {
    $('select.multiselect_sel').selectAll();
  });

  //moves selection if it's double clicked to selected box
  $('select.multiselect_unsel').dblclick(function() {
    unselclass = '.' + this.id + '_unsel';
    selclass = '.' + this.id + '_sel';
    $(unselclass).moveSelectionTo($(selclass));
  });

  //moves selection if it's double clicked to unselected box
  $('select.multiselect_sel').dblclick(function() {
    unselclass = '.' + this.id + '_unsel';
    selclass = '.' + this.id + '_sel';
    $(selclass).moveSelectionTo($(unselclass));
  });

  //moves selection if add is clicked to selected box
  $('span.multiselect_add').click(function() {
    unselclass = '.' + this.id + '_unsel';
    selclass = '.' + this.id + '_sel';
    $(unselclass).moveSelectionTo($(selclass));
  });

  //moves selection if remove is clicked to selected box
  $('span.multiselect_remove').click(function() {
    unselclass = '.' + this.id + '_unsel';
    selclass = '.' + this.id + '_sel';
    $(selclass).moveSelectionTo($(unselclass));
  });
});

//selects all the items in the select box it is called from.
//usage $('nameofselectbox').selectAll();
//
jQuery.fn.selectAll = function()
{
  this.each(function()
  {
    for (var i=0;i<this.options.length;i++)
    {
      option = this.options[i];
      option.selected = true;   
    }
  });
}

//removes the content of this select box from the target
//usage $('nameofselectbox').removeContentsFrom(target_selectbox)
//
jQuery.fn.removeContentsFrom = function()
{
  dest = arguments[0];
  this.each(function()
  {
    for (var i=this.options.length-1;i>=0;i--)
    {
      dest.removeOption(this.options[i].value);
    }
  });
}


//moves the selection to the select box specified
//usage $('nameofselectbox').moveSelectionTo(destination_selectbox)
//
jQuery.fn.moveSelectionTo = function()
{
  dest = arguments[0];
  this.each(function()
  {
    for (var i=this.options.length-1;i>=0;i--)
    {
      option = this.options[i];
      if (option.selected)
      {
        dest.addOption(option);
        this.remove(i);
      }
    }
  });
}

//Adds an option to a select box
//usage $('nameofselectbox').addOption(optiontoadd);
//
jQuery.fn.addOption = function()
{
  option = arguments[0];
  this.each(function()
  {
    //had to alter code to this to make it work in IE
    anOption = document.createElement('option');
    anOption.text = option.text;
    anOption.value = option.value;
    this.options[this.options.length] = anOption;
    return false;
  });
}

//Removes an option from a select box
//usage $('nameofselectbox').removeOption(valueOfOptionToRemove);
//
jQuery.fn.removeOption = function()
{
  targOption = arguments[0];

  this.each(function()
  {
    for (var i=this.options.length-1;i>=0;i--)
    {
      option = this.options[i];
      if (option.value==targOption)
      {
        this.remove(i);
      }
    }
  });
}

