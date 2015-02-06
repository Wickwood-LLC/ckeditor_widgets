<script type="text/javascript">var appUrl = '<?php print $app_url; ?>';</script>
<div class="angular-media-field" ng-app="app">
  <div ui-view></div>
  <div class="thumbnails" ng-class="" ng-controller="thumbnails" ng-init='init({
  apiUrl: "<?php print $api_url; ?>",
  fieldName: "<?php print $field_name; ?>",
  files: <?php print drupal_json_encode($files, JSON_HEX_APOS); ?>,
  multiple: <?php print (int)$multiple; ?>,
  flickrApiKey: "<?php print $flickr_key; ?>"
})'><!-- nv-file-drop="" uploader="uploader"-->
    
    <a href class="thumbnail" ng-repeat="(key, file) in files" title="{{file.name}}">
      <img ng-src="{{file.thumbUrl}}" />
      <div class="thumbnail-title">{{file.name}}</div>
        <button type="button" class="btn btn-default btn-xs thumbnail-edit" ng-click="item.remove()"><span class="glyphicon glyphicon-cog" aria-hidden="true"></span></button>
        <button type="button" class="btn btn-danger btn-xs thumbnail-remove" ng-click="item.remove(key)"><span class="glyphicon glyphicon-trash" aria-hidden="true"></span></button>
    </a>
    <div class="thumbnail drop-zone">
      <div><span class="glyphicon glyphicon-download" aria-hidden="true"></span> Drop files</div>
      <div class="or">or</div>
      <a class="btn btn-primary media-select" ui-sref="modal.upload"><span class="glyphicon glyphicon-picture" aria-hidden="true"></span> Select</a>
    </div>
  </div>

  <input type="hidden" name="<?php print $field_name; ?>_media" value="{{files}}" />
  
  <div class="clearfix"></div>
</div>