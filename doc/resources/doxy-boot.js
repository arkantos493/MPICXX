$( document ).ready(function() {

    $("div.headertitle").addClass("page-header");
    $("div.title").addClass("h1");

    $('li > a[href="files.html"] > span').before("<i class='fa fa-file'></i> ");
    $('li > a[href="hierarchy.html"] > span').before("<i class='fa fa-sitemap'></i> ");
    $('li > a[href="functions_type.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="functions_rela.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="namespacemembers.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="namespacemembers_func.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="namespacemembers_vars.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="namespacemembers_type.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="namespacemembers_enum.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="globals.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="globals_defs.html"] > span').before("<i class='fa fa-list'></i> ");

    $('li > a[href="index.html"] > span').before("<i class='fa fa-cog'></i> ");
    $('li > a[href="modules.html"] > span').before("<i class='fa fa-square'></i> ");
    $('li > a[href="namespaces.html"] > span').before("<i class='fa fa-bars'></i> ");
    $('li > a[href="annotated.html"] > span').before("<i class='fa fa-list-ul'></i> ");
    $('li > a[href="classes.html"] > span').before("<i class='fa fa-book'></i> ");
    $('li > a[href="inherits.html"] > span').before("<i class='fa fa-sitemap'></i> ");
    $('li > a[href="functions.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="functions_func.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="functions_vars.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="functions_enum.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="functions_eval.html"] > span').before("<i class='fa fa-list'></i> ");
    $('img[src="ftv2ns.png"]').replaceWith('<span class="label label-danger">N</span> ');
    $('img[src="ftv2cl.png"]').replaceWith('<span class="label label-danger">C</span> ');

    var ul_tablist = $("ul.tablist");
    ul_tablist.addClass("nav nav-pills nav-justified");
    ul_tablist.css("margin-top", "0.5em");
    ul_tablist.css("margin-bottom", "0.5em");
    $("li.current").addClass("active");
    $("iframe").attr("scrolling", "yes");

    $("#nav-path > ul").addClass("breadcrumb");

    $("table.params").addClass("table");
    $("div.ingroups").wrapInner("<span class='text-nowrap'></span>");
    $("div.levels").css("margin", "0.5em");
    $("div.levels > span").addClass("btn btn-default btn-xs");
    $("div.levels > span").css("margin-right", "0.25em");

    $("table.directory").addClass("table table-striped");
    $("div.summary > a").addClass("btn btn-default btn-xs");
    $("table.fieldtable").addClass("table");
    $(".fragment").addClass("well");
    $(".memitem").addClass("panel panel-default");
    $(".memproto").addClass("panel-heading");
    $(".memdoc").addClass("panel-body");
    $("span.mlabel").addClass("label label-info");

    $("table.memberdecls").addClass("table");
    $("[class^=memitem]").addClass("active");

    $("div.ah").addClass("btn btn-default");
    $("span.mlabels").addClass("pull-right");
    $("table.mlabels").css("width", "100%")
    $("td.mlabels-right").addClass("pull-right");

    $("div.ttc").addClass("panel panel-primary");
    $("div.ttname").addClass("panel-heading");
    $("div.ttname a").css("color", 'white');
    $("div.ttdef,div.ttdoc,div.ttdeci").addClass("panel-body");

	$('table.doxtable').removeClass('doxtable').addClass('table table-striped table-bordered').each(function(){
		$(this).prepend('<thead></thead>');
		$(this).find('tbody > tr:first').prependTo($(this).find('thead'));

		$(this).find('td > span.success').parent().addClass('success');
		$(this).find('td > span.warning').parent().addClass('warning');
		$(this).find('td > span.danger').parent().addClass('danger');
	});



    if($('div.fragment.well div.ttc').length > 0)
    {
        $('div.fragment.well div.line:first').parent().removeClass('fragment well');
    }

    $('table.memberdecls').find('.memItemRight').each(function(){
        $(this).contents().appendTo($(this).siblings('.memItemLeft'));
        $(this).siblings('.memItemLeft').attr('align', 'left');
    });

    $('table.memberdecls').find('.memTemplItemRight').each(function(){
        $(this).contents().appendTo($(this).siblings('.memTemplItemLeft'));
        $(this).siblings('.memTemplItemLeft').attr('align', 'left');
    });

	function getOriginalWidthOfImg(img_element) {
		var t = new Image();
		t.src = (img_element.getAttribute ? img_element.getAttribute("src") : false) || img_element.src;
		return t.width;
	}

	$('div.dyncontent').find('img').each(function(){
		if(getOriginalWidthOfImg($(this)[0]) > $('#content>div.container').width())
			$(this).css('width', '100%');
	});


  /* responsive search box */
  $('#MSearchBox').parent().remove();

  var nav_container = $('<div class="row"></div>');
  $('#navrow1').parent().prepend(nav_container);

  var left_nav = $('<div class="col-md-9"></div>');
  for (i = 0; i < 6; i++) {
    var navrow = $('#navrow' + i + ' > ul.tablist').detach();
    left_nav.append(navrow);
    $('#navrow' + i).remove();
  }
  var right_nav = $('<div class="col-md-3"></div>').append('\
    <div id="search-box" class="input-group">\
      <div class="input-group-btn">\
        <button aria-expanded="false" type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown">\
          <span class="glyphicon glyphicon-search"></span> <span class="caret"></span>\
        </button>\
        <ul class="dropdown-menu">\
        </ul>\
      </div>\
      <button id="search-close" type="button" class="close" aria-label="Close"><span aria-hidden="true">&times;</span></button>\
      <input id="search-field" class="form-control" accesskey="S" onkeydown="searchBox.OnSearchFieldChange(event);" placeholder="Search ..." type="text">\
    </div>');
  $(nav_container).append(left_nav);

  var header_container = document.getElementsByClassName("navbar-header");
  $(header_container).append(right_nav);

  $('#MSearchSelectWindow .SelectionMark').remove();
  var search_selectors = $('#MSearchSelectWindow .SelectItem');
  for (var i = 0; i < search_selectors.length; i += 1) {
    var element_a = $('<a href="#"></a>').text($(search_selectors[i]).text());

    element_a.click(function(){
      $('#search-box .dropdown-menu li').removeClass('active');
      $(this).parent().addClass('active');
      searchBox.OnSelectItem($('#search-box li a').index(this));
      searchBox.Search();
      return false;
    });

    var element = $('<li></li>').append(element_a);
    $('#search-box .dropdown-menu').append(element);
  }
  $('#MSearchSelectWindow').remove();

  $('#search-box .close').click(function (){
    searchBox.CloseResultsWindow();
  });

  $('body').append('<div id="MSearchClose"></div>');
  $('body').append('<div id="MSearchBox"></div>');
  $('body').append('<div id="MSearchSelectWindow"></div>');

  searchBox.searchLabel = '';
  searchBox.DOMSearchField = function() {
    return document.getElementById("search-field");
  }
  searchBox.DOMSearchClose = function(){
    return document.getElementById("search-close");
  }


  /* search results */
  var results_iframe = $('#MSearchResults').detach();
  $('#MSearchResultsWindow')
    .attr('id', 'search-results-window')
    .addClass('panel panel-default')
    .append(
      '<div class="panel-heading">\
        <h3 class="panel-title">Search Results</h3>\
      </div>\
      <div class="panel-body"></div>'
    );
  $('#search-results-window .panel-body').append(results_iframe);

  searchBox.DOMPopupSearchResultsWindow = function() {
    return document.getElementById("search-results-window");
  }

  function update_search_results_window() {
    $('#search-results-window').removeClass('panel-default panel-success panel-warning panel-danger')
    var status = $('#MSearchResults').contents().find('.SRStatus:visible');
    if (status.length > 0) {
      switch(status.attr('id')) {
        case 'Loading':
        case 'Searching':
          $('#search-results-window').addClass('panel-warning');
          break;
        case 'NoMatches':
          $('#search-results-window').addClass('panel-danger');
          break;
        default:
          $('#search-results-window').addClass('panel-default');
      }
    } else {
      $('#search-results-window').addClass('panel-success');
    }
  }
  $('#MSearchResults').load(function() {
    $('#MSearchResults').contents().find('link[href="search.css"]').attr('href','../doxygen.css');
    $('#MSearchResults').contents().find('head').append(
      '<link href="customdoxygen.css" rel="stylesheet" type="text/css">');

    update_search_results_window();

    // detect status changes (only for search with external search backend)
    var observer = new MutationObserver(function(mutations) {
      update_search_results_window();
    });
    var config = { attributes: true};

    var targets = $('#MSearchResults').contents().find('.SRStatus');
    for (i = 0; i < targets.length; i++) {
      observer.observe(targets[i], config);
    }
  });


  /* enumerations */
  $('table.fieldtable').removeClass('fieldtable').addClass('table table-striped table-bordered').each(function(){
    $(this).prepend('<thead></thead>');
    $(this).find('tbody > tr:first').prependTo($(this).find('thead'));

    $(this).find('td > span.success').parent().addClass('success');
    $(this).find('td > span.warning').parent().addClass('warning');
    $(this).find('td > span.danger').parent().addClass('danger');
  });

  /* todo list */
  var todoelements = $('.contents > .textblock > dl.reflist > dt, .contents > .textblock > dl.reflist > dd');
  for (var i = 0; i < todoelements.length; i += 2) {
    $('.contents > .textblock').append(
      '<div class="panel panel-default active">'
        + "<div class=\"panel-heading todoname\">" + $(todoelements[i]).html() + "</div>"
        + "<div class=\"panel-body\">" + $(todoelements[i+1]).html() + "</div>"
      + '</div>');
  }
  $('.contents > .textblock > dl').remove();


	$(".memitem").removeClass('memitem');
    $(".memproto").removeClass('memproto');
    $(".memdoc").removeClass('memdoc');
	$("span.mlabel").removeClass('mlabel');
	$("table.memberdecls").removeClass('memberdecls');
    $("[class^=memitem]").removeClass('memitem');
    $("span.mlabels").removeClass('mlabels');
    $("table.mlabels").removeClass('mlabels');
    $("td.mlabels-right").removeClass('mlabels-right');
	$(".navpath").removeClass('navpath');
	$("li.navelem").removeClass('navelem');
	$("a.el").removeClass('el');
	$("div.ah").removeClass('ah');
	$("div.header").removeClass("header");

	$('.mdescLeft').each(function(){
		if($(this).html()=="&nbsp;") {
			$(this).siblings('.mdescRight').attr('colspan', 2);
			$(this).remove();
		}
	});
  $('td.memItemLeft').each(function(){
    if($(this).siblings('.memItemRight').html()=="") {
      $(this).attr('colspan', 2);
      $(this).siblings('.memItemRight').remove();
    }
  });
	$('td.memTemplItemLeft').each(function(){
		if($(this).siblings('.memTemplItemRight').html()=="") {
			$(this).attr('colspan', 2);
			$(this).siblings('.memTemplItemRight').remove();
		}
	});
  searchBox.CloseResultsWindow();

  // replace newline in front of code snippets with empty string
  $(".line").each(function(){
      $(this).html($(this).html().replace(/&nbsp;/gi,''));
  });

  // add table class to tparams
  $("table.tparams").addClass("table");
  $("table.exception").addClass("table");

  // change custom group from one dd to n dds
  ["assert_precondition", "assert_sanity"].forEach(function(elem) {
      [].forEach.call(document.getElementsByClassName(elem), function(item) {
           var assertions = item.lastElementChild.innerHTML.split("\n");
           item.lastElementChild.remove();
           assertions.forEach(function(a, i) {
               var dd = document.createElement("DD");
               var template = document.createElement("template");
               template.innerHTML = a.trim();
               dd.appendChild(template.content);
               item.appendChild(dd);
           });
      });
  });

  // correctly place nodiscard
  [].forEach.call(document.getElementsByClassName("nodiscard"), function(item) {
      var parent_panel = item.closest(".panel");
      var target_child = $(parent_panel).find("span.pull-right").first();
      target_child.append(item);
  });

  // switch themes
  var toggleSwitch = document.querySelector('.theme-switch input[type="checkbox"]');
  function switchTheme(e) {
      if (e.target.checked) {
          document.documentElement.setAttribute('data-theme', 'dark');
          localStorage.setItem('theme', 'dark'); //add this
      } else {
          document.documentElement.setAttribute('data-theme', 'light');
          localStorage.setItem('theme', 'light'); //add this
      }
  }
  toggleSwitch.addEventListener('change', switchTheme, false);

  var currentTheme = localStorage.getItem('theme') ? localStorage.getItem('theme') : null;
  if (currentTheme) {
      document.documentElement.setAttribute('data-theme', currentTheme);
      if (currentTheme === 'dark') {
          toggleSwitch.checked = true;
      }
  }

  // better inline code link style
  $("a").has("code").addClass("inline_code_link");

  // better pages
  var current_page = window.location.pathname.split("/").pop();
  var supported_pages = [ "namespacemembers.html", "namespacemembers_func.html", "functions_func.html" ];
  if (supported_pages.includes(current_page)) {
      // hide elements
      $("div.contents>ul, div.contents>h3").css("display", "none");

      var navbar = $("div.col-md-9").children()[3];

      navbar.addEventListener("click", function(e) {
          // set navbar
          $(navbar).find("li.current").removeClass();
          var current = $(e.target).closest("li");
          current.addClass("current active");

          // hide old element
          $("div.contents>ul, div.contents>h3").css("display", "none");

          // show new element
          var selector = "div.contents>h3:contains(- " + current.text() + " -)";
          $(selector).css("display", "block");
          $(selector).next().css("display", "block");
      });

      $(navbar).find("li").first().click();
  }

  // fix table on readme page
  if (current_page === "index.html") {
      $("td").css("white-space", "normal");
      $("td").css("vertical-align", "middle");
      $("th").css("text-align", "center");
  }

  // remove spaces from empty parameter list
  $("td.paramname:empty").remove();
  // add whitespace between parameter list and "const"
  $(".memname td:contains(')')").append("&nbsp;");

  // remove requires(...) from brief function description
  $("td.memTemplItemLeft").each(function() {
      var text = $(this).html();
      $(this).html(text.substring(0, text.indexOf("requires")));
  });

  // correctly display C++20 concepts
  $("span.concept").each(function() {
      $(this).closest("tr").prev("tr").find("td.memTemplItemLeft").html($(this).html());
      $(this).remove();
  });
});