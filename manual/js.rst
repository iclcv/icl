.. raw:: html
  
  <script src="../js/jquery.min.js"></script>
  <script src="../js/jquery.tools.min.js"></script>
  <!--script src="../_static/underscore.js"></script-->
         <!--script src="../_static/doctools.js"></script-->


  <script>
  $(document).ready(function() {
 
    console.log('Im being executed!');

    // works  $('a').after('<div class="tooltip"> Hello World </div>')
    $('.reference.external').after(function() {
      var href = this.href;
      var text = this.text;

      var package = 'unknown';
      var type = 'other';


      if( this.text.match('^ICL.*\.h$') ){ // we have a header file
         var m = this.text.match('ICL(\[^/\]*)/.*');
         if( m ){
            package = m[1].toLowerCase();
            type = "header";
         }
      }
      
      var packages = [ 'utils', 'math', 'core', 'filter', 'io', 'qt', 'cv', 'geom', 'markers' ];
      var groupLUT = [ ['TIME', 'EXCEPT', 'THREAD', 'RANDOM', 'UTILS' , 'PA', 'XML', 'STRUTILS', 'FUNCTION', 'BASIC__TYPES'],
                       ['LINALG'],
                       ['TYPES', 'GENERAL', 'IMAGE'],
                       ['UNARY', 'BINARY', 'AFFINE', 'NBH', 'INPLACE' ],
                       ['DC_G', 'UTILS_G', 'FILEIO_G', 'MOVIE_FILE_G', 'V4L_G', 'GIGE_G'],
                       ['COMMON', 'HANDLES', 'UNCOMMON'],
                       ['G_RD'],
                       [],
                       ['PLUGINS']
                     ];

      if(package == 'unknown'){
        for(var i=0;i<9;++i){
          if( href.match('.*/namespaceicl_1_1'+packages[i]+'\.html') ){
            package = packages[i];
            // could be a function or a namespace
            if(this.text.match('.*'+packages[i]+'$')){
               type = 'namespace';
            }else if(this.text[0] > 'A' && this.text[0] < 'Z'){
               type = 'global type';
            }else{
               type = 'global function';
            }
            break;
          }
          if( href.match('.*icl_1_1'+packages[i]+'.*') ){
            package = packages[i]; 
            break;
          }
        }
      }
      if(package == 'unknown'){
        var res = href.match('.*group__(\[^.\]*).*')
        if( res ){
           var groupName = res[1];
           for(var i=0;i<9;++i){
              if(groupLUT[i].indexOf(groupName) != -1){
                  package = packages[i];
                  break;
              }
           }
           var res2 = this.text.match('.*::(\[^:\]*)$');
           var t = "???";
           if(res2){
              t = res2[1];
           }else{
              t = this.text;
           }
           if(t[0] > 'A' && t[0] < 'Z'){
              type = "grouped type";
           }else{
              if(groupName == "TYPES"){
                type = "core type";
              }else if(groupName == "BASIC__TYPES"){
                type = "basic type";
              }else{
                type = "grouped function";
              }
           }
         }
      }
      
      if(href.match('.*classicl.*')){
         if(href.match('.*#\[0-9a-f\]*')){
           type = 'class: method';
         }else{
           type = 'class';
         }
      }else if(href.match('.*structicl.*')){
         if(href.match('.*#\[0-9a-f\]*')){
           type = 'struct: method';
         }else{
           type = 'struct';
         }
      }
      
      if(package == "unknown" && href.match('.*namespaceicl.*')){
        // very special treatment
        return '<div class="tooltip">the <b>icl</b> namespace is used for all '
              +'modules.</div>';
    
      }else if(package != "unknown"){
        return '<div class="tooltip">' 
           + '<a href="../modules/'+package+'.html">'
           + '<img title="manual: '+package+' module" width="110px" src="../_images/'+package+'1.png"></img>'
           + '</a>'
           + '<br/>' + 'Type:   <b>' + type + '</b>' 
           + '</div>';
      }else{
        return '<div class="tooltip">' 
           + 'Unable to locate package'
           + '<br/>' + 'Type:   <b>' + type + '</b>' 
           + '</div>';
      }

      //return '<div class="tooltip">' + 'TEST TEST TEST' + '</div>';
    });

    $('.reference.external').tooltip({
      position: "top center", 
      opacity: 0.95, 
      effect: 'fade',
      offset: [7,33]
    });
  });
  </script>
  <style type="text/css">
    .tooltip {
      /*border: 1px solid #999; */
      /*border-radius:5px; */
      display:none;
      /*background: rgb(255,255,255); /*transparent url(images/white_arrow.png);*/
      background: transparent url(../_images/tooltip.png);
      font-size:13px;
      height:80px;
      width:131px;
      padding:10px;
      color: #555;
      /*box-shadow: 4px 4px 12px rgba(0,0,0,0.5); */
      line-height: 20px;
    }
    a.reference.external {
      color: rgb(20,60,100);
      padding: 2px;
      padding-left: 6px;
      padding-right: 5px;
    }
    a.reference.external:hover {
      color: rgb(20,60,100);
      border: 1px solid rgba(0,0,0,0.3);
      border-radius: 5px;
      box-shadow: 2px 2px 6px rgba(0,0,0,0.2);
      padding: 2px;
      padding-left: 5px;
      padding-right: 4px;
      text-decoration: none;
    }

   div.body{
      border-top-left-radius: 15px;
      border-bottom-left-radius: 15px;
      border: 1px solid rgb(110,110,110);
      box-shadow: 0px 0px 50px rgba(0,0,0,0.7);
    }

    div.body h1{
      border-top-left-radius: 15px;
      box-shadow: 0px 2px 0px rgba(0,0,0,0.4);
    }

    div.body h2{
      box-shadow: 0px 2px 0px rgba(0,0,0,0.4);
    }

    div.body h3{
      box-shadow: 0px 2px 0px rgba(0,0,0,0.4);
    }
    div.sphinxsidebar{
      font-size: 80%;
    }

    table.docutils td, table.docutils th{
      border: 0px;
    }
    th {
       background-color: #0F67A1;
       color: rgb(220,220,220);
    }
  </style>
