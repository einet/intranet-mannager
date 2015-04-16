


var i = 0,
    duration = 750,
    root;

var tree = d3.layout.tree()
    .children(function (d) {
      if (!d.hidesChildren && d.children && d.collapsed && d.children.length) {
        d.hidesChildren = true
      }
      return d.collapsed ? null : d.children
    })
    .size([height, width]);

var diagonal = d3.svg.diagonal()
    .projection(function(d) { return [d.y, d.x]; });

var svg = d3.select("#d3view").append("svg")
    .attr("width", width + margin.right + margin.left)
    .attr("height", height + margin.top + margin.bottom)
    .append("g")
    .attr("transform", "translate(" + margin.left + "," + margin.top + ")");


var listed
var root
var conved 
function start(flare) 
{
  root = flare;
  
  conved = make_listed(flare, undefined, true)
    , main = document.getElementById('editme');

  conved.tree[conved.id].collapsed = false;
 
  listed = new Listed(conved.id, conved.tree, main)

  function setCollapsed(id, doCollapse) {
    listed.ctrl.setCollapsed(id, doCollapse)
    listed.ctrl.view.startEditing(id)
  }

  root = listed.ctrl.model.dumpData()
  root.x0 = height / 2;
  root.y0 = 0;
  update(root, setCollapsed);

  listed.ctrl.on('change', function () {
    var root = listed.ctrl.model.dumpData()
    root.x0 = height / 2;
    root.y0 = 0;
    update(root, setCollapsed)
  })
}

d3.select(self.frameElement).style("height", "800px");

var COLORS = {
  done: '#0f0',
  parent: 'lightsteelblue'
}
var keymsg;
function savePageAs(fileName){

$(".export").attr({
            'download': fileName,
            'href': "data:text/html;charset=utf-8," + keymsg,
            'target': '_blank'
        });
  // return window.open( "data:x-application/external;charset=utf-8," + keymsg);
}//end savePageAs


function save_record(filename) 
{ 
  $('<div></div>').appendTo('body')
  .html('<div><h6>配置文件内容如下:<br>'+ 'var flare_data='+keymsg+';</h6></div>')
  .dialog({
      modal: true, title: '配置文件', zIndex: 10000, autoOpen: true,
      width: 'auto', resizable: true,
      buttons: {
          "关闭": function () {
              $(this).dialog("close");
          }
      },
      close: function (event, ui) {
          $(this).remove();
      }
  });
}
function update(source, setCollapsed) {

  keymsg = $.toJSON(source);
  
 // Compute the new tree layout.
  var nodes = tree.nodes(source).reverse(),
      links = tree.links(nodes);

  // Normalize for fixed-depth.
  nodes.forEach(function(d) { d.y = d.depth * 180; });

  // Update the nodes…
  var node = svg.selectAll("g.node")
       .data(nodes, function(d) {
         return d.id || (d.id = ++i);
       });

  // Enter any new nodes at the parent's previous position.
  var nodeEnter = node.enter().append("g")
      .attr("class", "node")
      .attr("transform", function(d) {
        var _source = d.parent || source
        return "translate(" + _source.y + "," + _source.x + ")";
      })
      ;

  nodeEnter.append("circle")
      .attr("r", 1e-6)
      .style('stroke', function (d) {
        return d.hidesChildren ? '' : (d.done ? COLORS.done : '')
      })
      .style("fill", function(d) {
        if (d.done) return COLORS.done
        return d.hidesChildren ? COLORS.parent: "#fff";
      })
      .on("click", click);

  nodeEnter.append("text")
      .attr("x", function(d) { return d.hidesChildren ? -10 : 10; })
      .attr("dy", ".35em")
      .attr("text-anchor", function(d) { return d.hidesChildren ? "end" : "start"; })
      .text(function(d) { return d.name; })
      .style("fill-opacity", 1e-6)
      .on("dblclick", dblclick);

  // Transition nodes to their new position.
  var nodeUpdate = node.transition()
      .duration(duration)
      .attr("transform", function(d) {
        return "translate(" + d.y + "," + d.x + ")";
      });

  nodeUpdate.select("circle")
      .attr("r", 8.5)
      .style('stroke', function (d) {
        return d.hidesChildren ? '' : (d.done ? COLORS.done : '')
      })
      .style("fill", function(d) {
        if (d.done) return COLORS.done
        return d.hidesChildren ? COLORS.parent: "#000";
      });

  nodeUpdate.select("text")
      .text(function(d) { return d.name; })
      .style("fill-opacity", 1);

  // Transition exiting nodes to the parent's new position.
  var nodeExit = node.exit().transition()
      .duration(duration)
      .attr("transform", function(d) {
        return "translate(" + d.parent.y + "," + d.parent.x + ")";
      })
      .remove();

  nodeExit.select("circle")
      .attr("r", 1e-6);

  nodeExit.select("text")
      .style("fill-opacity", 1e-6);

  // Update the links…
  var link = svg.selectAll("path.link")
      .data(links, function(d) { return d.target.id; });

  // Enter any new links at the parent's previous position.
  link.enter().insert("path", "g")
      .attr("class", "link")
      .attr("d", function(d) {
        var source = d.source
        var o = {x: source.x, y: source.y};
        return diagonal({source: o, target: o});
      });

  // Transition links to their new position.
  link.transition()
      .duration(duration)
      .attr("d", diagonal);

  // Transition exiting nodes to the parent's new position.
  link.exit().transition()
      .duration(duration)
      .attr("d", function(d) {
        var o = {x: d.source.x, y: d.source.y};
        return diagonal({source: o, target: o});
      })
      .remove();

  // Stash the old positions for transition.
  nodes.forEach(function(d) {
    d.x0 = d.x;
    d.y0 = d.y;
  });
  var last;
   function openchatdialog(str)
  {
    if ($("#dialog").length==0) {
      return;
    }
    //dialog options
    if (last !=undefined && last !=null) {
      last.dialog('open');
      $(".ui-dialog-title")[0].innerHTML = "和服务器聊天:" + str;
      $("#dialog").dialogExtend("maximize");
      return;
    }
    
    var dialogOptions = {
      "title" : "和服务器聊天:" + str,
      "width" : 1300,
      "height" : 800,
      "modal" : false,
      "resizable" : true,
      "draggable" : true
    };
    if ( $("#button-cancel").is(":checked") ) {
      dialogOptions.buttons = { "Cancel" : function(){ $(this).dialog("close"); } };
    }
    // dialog-extend options
    var dialogExtendOptions = {
      "closable" : true,
      "maximizable" : true,
      "minimizable" : true,
      "collapsable" : true
    };
   
    // open dialog
    last = $("#dialog").dialog(dialogOptions).dialogExtend(dialogExtendOptions);
    $("#dialog").dialogExtend("maximize");
  }
 var hostname="";

 function gethostname(d)
 {
	if (d.children != undefined) {
	  for(var j=0;j<d.children.length;j++)
	  {
	    gethostname(d.children[j]);
	  }
	}else
	{
	  if(d.ip =="" ||d.ip==null || d.ip==undefined)
	    return;
	  else
	  {
	    hostname += d.name;
	    hostname += " ";

	  }
	}
	
 }
  function dblclick(d) {
    hostname = "";
    if(d.depth == 0)
    {
	d.name = "all";
	d.ip   = "所有主机"
	hostname = "all";
     }else		
     {
	 gethostname(d);
     }
     if (hostname.length ==0) {
		$("#Tishi")[0].innerText = "请选择可以联系的服务器！";
		return;
     }
    
    if($("#catinfo",window.parent.document).length>0)
    {
       	window.parent.closedialog();
    }
    $("#Tishi")[0].innerText = "查询主机:"+hostname;
    $.cookie('sethostname',hostname,{expires: 24*60*60});
    $.post("/chat/sendmsg.asp",$.toJSON({sendmsg:"set host clear"}),function(result)
    {    
	jmsg = $.parseJSON(result);
	$.post("/chat/sendmsg.asp",$.toJSON({sendmsg:"set host "+hostname}),function(result)
	{    
	    jmsg = $.parseJSON(result);
	});
    });
    
    openchatdialog(hostname);
  }

  // Toggle children on click.
  function click(d) {
    setCollapsed(d.id, !d.collapsed);
    }
}
start(flare_data);

