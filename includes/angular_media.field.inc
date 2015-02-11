<?php

/**
 * Implements hook_field_widget_info().
 */
function angular_media_field_widget_info() {
  return array(
    'file_angular_media' => array(
      'label' => t('Angular Media Dialog'),
      'field types' => array('file', 'image', 'media'),
      //'settings' => array('size' => 60),
      'behaviors' => array(
        'multiple values' => FIELD_BEHAVIOR_CUSTOM, //Value for field API indicating a widget can receive several field values.
        'default value' => FIELD_BEHAVIOR_NONE,
      ),
    ),
  );
}

/**
 * Implements hook_field_widget_form().
 */
function angular_media_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  $options = array(
    'cardinality' => $field['cardinality'],
    'field_name' => $instance['field_name'],
    'files' => array(),
  );
  foreach ($items as $item) {
    $options['files'][] = angular_media_simplify_file($item);
  }

  $path = angular_media_app_path();
  $form['#attached'] = array(
    // @todo: make libraries_load work
    //'libraries_load' => array(
    //  array('angular_media', 'angular-media-app'),
    //)
    'js' => array(
      $path . 'js/libraries.min.js' => array(),
      $path . 'js/app.min.js' => array(),
      //$path . 'js/vendor/jcrop.min.js' => array('weight' => 10),
      drupal_get_path('module', 'angular_media') . '/js/angular-media.js' => array(),
    ),
    'css' => array(
      $path . 'css/app.min.css' => array(),
    ),
  );
  $element += array(
    '#type' => 'textfield',
    '#title' => $instance['label'],
    '#required' => $instance['required'],
    '#suffix' => theme('angular_media_app', $options),
    '#attributes' => array('style' => 'display:none'),
  );

  //@todo: this is hacky
  $form['#validate'][] = 'angular_media_field_widget_form_validate';
  $form['#submit'][] = 'angular_media_field_widget_form_validate';
  $form['angular_media_fields'] = isset($form['angular_media_fields']) ? $form['angular_media_fields'] : array('#type' => 'value', '#value' => array());
  $form['angular_media_fields']['#value'][] = $instance['field_name'];

  return array($element);
}


/**
 * An element #process callback.
 *
 * Parses and savesthe angular-media-app json.
 */
function angular_media_field_widget_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['angular_media_fields'])) {
    foreach ($form_state['values']['angular_media_fields'] as $field_name) {
      $json = json_decode($_POST[$field_name.'_media']);
      $form_state['values'][$field_name][LANGUAGE_NONE] = array();
      //$form_state['values']['field_media2'] = $form_state['values']['field_media'];
      foreach ($json as $key => $item) {
        $item->display = 1;
        $item->uplodate = '';
        $form_state['values'][$field_name][LANGUAGE_NONE][] = (array)$item;
      }
      dpm($form_state['values'][$field_name]);
    }
  }
}


/******************************* FIELD FORMATTERS ***************************** */

/**
* Implements hook_field_formatter_info().
*/
function angular_media_field_formatter_info() {
  $info = array(
    'image_style_url' => array(
      'label' => t('URL to image style'),
      'field types' => array('file', 'image'),
      'settings'  => array(
        'image_style' => '',
      ),
    ),
    'image_super' => array(
      'label' => t('Super Image'),
      'field types' => array('file', 'image'),
      'settings'  => array(
        'image_style' => '',
        'link_field' => '',
        'hide_if_field' => '',
        'image_style_medium' => '',
        'image_style_large' => '',
        'image_style_retina' => '',
        'placeholder_dimensions' => '',
        'placeholder_text' => '',
      ),
    ),
    'image_link' => array(
      'label' => t('Image linking to another field'),
      'field types' => array('file', 'image'),
      'settings'  => array(
        'image_style' => '',
        'link_field' => '',
      ),
    ),
    'file_click_to_play' => array(
      'label' => t('Click-to-play'),
      'field types' => array('file'),
      'settings'  => array(
        'format' => 'inline',
        'image_field' => 'thumb',
        'image_style' => '',
        'autoplay' => TRUE,
        'class' => '',
        'play_class' => 'button icon-play',
      ),
    ),
  );
  return $info;
}


function angular_media_field_info_alter(&$info) {
  //dpm($info);
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function angular_media_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];

  // Add fields
  switch ($display['type']) {
    case 'image_link':
      $element['link_field'] = array(
        '#title' => t('Link field'),
        '#type' => 'select',
        '#default_value' => $settings['link_field'],
        '#options' => _angular_media_fields('link_field'),
      );
      break;

    case 'file_click_to_play':
      $element['format'] = array(
        '#title' => t('format'),
        '#type' => 'radios',
        '#default_value' => $settings['format'],
        '#options' => array(
          'inline' => 'Play video in-place',
          'modal' => 'Play video in a Foundation Modal.'
        ),
      );
      $element['image_field'] = array(
        '#title' => t('Thumbnail image field'),
        '#type' => 'select',
        '#default_value' => $settings['image_field'],
        '#options' => _angular_media_fields(array('file', 'image'), array('oembed' => t('- Use oEmbed thumbnail -'))),
        '#description' => t('Select the image field to use for the static thumbnail image. If you would like to use the thubnail from the 3rd party service (Youtube, etc), select "Use oEmbed thumbnail".'),
      );
      $image_styles = image_style_options(FALSE, PASS_THROUGH);
      $element['image_style'] = array(
        '#title' => t('Thumbnail image style'),
        '#type' => 'select',
        '#default_value' => $settings['image_style'],
        '#empty_option' => t('None (do not include this)'),
        '#options' => $image_styles,
        '#description' => t('This is only relevant if a Thumbnail image field is selected above.'),
      );
      $element['autoplay'] = array(
        '#title' => t('Autoplay'),
        '#type' => 'checkbox',
        '#default_value' => $settings['autoplay'],
        '#description' => t('Attempt to autoplay the video when clicked. Currently only works with Youtube.'),
      );
      $element['class'] = array(
        '#title' => t('Class'),
        '#type' => 'textfield',
        '#default_value' => $settings['class'],
        '#description' => t('Additonal class to go on the video when played. For modals, this can be used to adjust the size of the modal (and video, because it is wrapped in flex-video). See the !docs.', array('!docs' => l('Foundation reveal docs under Advanced', 'http://foundation.zurb.com/docs/components/reveal.html'))),
      );
      $element['play_class'] = array(
        '#title' => t('Play button class(es)'),
        '#type' => 'textfield',
        '#default_value' => $settings['play_class'],
      );
      break;

    case 'image_super':
      $element['markup'] = array(
        '#markup' => '<div>' . t('For details, see the !docs.', array('!docs' => l('Foundation docs', 'http://foundation.zurb.com/docs/components/interchange.html'))) . '</div>',
        '#weight' => -2,
      );
      $image_styles = image_style_options(FALSE, PASS_THROUGH);
      $element['image_style_medium'] = array(
        '#title' => t('Medium image style'),
        '#type' => 'select',
        '#default_value' => $settings['image_style_medium'],
        '#empty_option' => t('None (do not include this)'),
        '#options' => $image_styles,
      );
      $element['image_style_large'] = array(
        '#title' => t('Large image style'),
        '#type' => 'select',
        '#default_value' => $settings['image_style_large'],
        '#empty_option' => t('None (do not include this)'),
        '#options' => $image_styles,
      );
      $element['image_style_retina'] = array(
        '#title' => t('Retina image style'),
        '#type' => 'select',
        '#default_value' => $settings['image_style_retina'],
        '#empty_option' => t('None (do not include this)'),
        '#options' => $image_styles,
      );
      $element['link_field'] = array(
        '#title' => t('Link field'),
        '#type' => 'select',
        '#default_value' => $settings['link_field'],
        '#options' => _angular_media_fields('link_field', array('' => t('- None -'))),
      );
      $element['hide_if_field'] = array(
        '#title' => t('Hide if this field has a value'),
        '#type' => 'select',
        '#default_value' => $settings['hide_if_field'],
        '#options' => _angular_media_fields(array('file', 'image'), array('' => t('- None -'))),
        '#description' => T('Only show this image if this field does not have a value.  This is useful to ensure that two media items never get displayed on the same page, or to use this field as a Thumbnail field for a Click-to-play media item.'),
      );
      $element['placeholder_dimensions'] = array(
        '#title' => t('Placeholder dimensions'),
        '#type' => 'textfield',
        '#default_value' => $settings['placeholder_dimensions'],
        '#description' => T('For example: 200x100'),
      );
      $element['placeholder_text'] = array(
        '#title' => t('Placeholder text'),
        '#type' => 'textfield',
        '#default_value' => $settings['placeholder_text'],
        '#description' => 'For example: " "',
      );
      break;
  }

  // Add the image style select to those fields that need it
  if (
    $display['type'] === 'image_link' || 
    $display['type'] === 'image_video_popup' || 
    $display['type'] === 'image_link' || 
    $display['type'] === 'image_super'
  ) {
    $image_styles = image_style_options(FALSE, PASS_THROUGH);
    $element['image_style'] = array(
      '#title' => t('Image style'),
      '#type' => 'select',
      '#default_value' => $settings['image_style'],
      '#empty_option' => t('None (original image)'),
      '#options' => $image_styles,
      '#weight' => -1,
    );
  }
  
  return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 * @todo
 */
function angular_media_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];

  $summary = array();

  foreach ($settings as $key => $value) {
    $summary[] = $key . ': ' . $value;
  }
  return implode('<br />', $summary);
}


/**
 * Implements hook_field_formatter_view().
 */
function angular_media_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  $settings = $display['settings'];
    
  foreach ($items as $delta => $item) {
    
    switch ($display['type']) {
      case 'image_style_url':
        $uri = array(
          'path' => file_create_url(),
          'options' => array(),
        );
        $element[$delta] = array(
          '#markup' =>  $item['uri'] ?  image_style_url($settings['image_style'], $item['uri']) : '',
        );
        break;

      case 'image_link':
        if ($item['uri']) {
          $item['path'] = $item['uri'];
          $item['style_name'] = $settings['image_style'];
          $markup = theme('image_style', $item);
          if (!empty($entity->{$settings['link_field']})) {
            $wrapper = entity_metadata_wrapper($entity_type, $entity);
            $markup = l($value, $wrapper->{$settings['link_field']}->url->value(), array('html' => TRUE));
          }
        }
        else {
          $markup = '';
        }
        $element[$delta] = array(
          '#markup' => $markup,
        );
        break;

      case 'image_super':
        if ($item['uri']) {
          if (empty($settings['hide_if_field']) || empty($entity->{$settings['hide_if_field']})) {
            $markup = theme('image_interchange_style', array_merge($item, array(
              'path' => $item['uri'], 
              'image_style' => $settings['image_style'],
              'image_style_medium' => $settings['image_style_medium'],
              'image_style_large' => $settings['image_style_large'],
              'image_style_retina' => $settings['image_style_retina'],
            )));
            if (!empty($settings['link_field']) && !empty($entity->{$settings['link_field']})) {
              $wrapper = entity_metadata_wrapper($entity_type, $entity);
              $markup = l($markup, $wrapper->{$settings['link_field']}->url->value(), array('html' => TRUE));
            }
          }
        }
        if (!empty($markup)) {
          $element[$delta] = array(
            '#markup' => $markup,
          );
        }
        break;

      case 'file_click_to_play':
        // We currently only support oembed fields
        // @todo: Generalize this to add support for video fields too.
        if (
          !empty($item['metadata']['oembed'])
          && (!empty($item['metadata']['oembed']['thumbnail_url']) || (!empty($settings['link_field']) && !empty($entity->{$settings['link_field']} )))
        ) {
          // Get the output of the thumbnail from the image field
          if (!empty($settings['image_field']) && !empty($entity->{$settings['image_field']})) {
            // @todo support multilingual (use $wrapper?)
            $markup = theme('image_interchange_style', array_merge($entity->{$settings['image_field']}[LANGUAGE_NONE][0], array(
              'path' => $entity->{$settings['image_field']}[LANGUAGE_NONE][0]['uri'], 
              'image_style' => $settings['image_style'],
              'image_style_medium' => $settings['image_style_medium'],
              'image_style_large' => $settings['image_style_large'],
              'image_style_retina' => $settings['image_style_retina'],
            )));
          }
          // Get the output of the thumbnail from the oembed thumbnail field
          else {
            $item['attributes']['src'] = $item['metadata']['oembed']['thumbnail_url'];
            $markup = theme('html_tag', array('element' => array(
              '#tag' => 'img',
              '#attributes' => $item['attributes'],
            )));
          }
          // Now build out the tags
          if (!empty($markup)) {
            $content = $item['metadata']['oembed']['html'];
            $content = $settings['autoplay'] ? str_replace('oembed"', 'oembed&autoplay=1"', $content) : $content;
            $element[$delta] = array(
              '#theme' => 'angular_media_clicktoplay_' . $settings['format'],
              '#placeholder' => $markup,
              '#play_class' => $settings['play_class'],
              '#content' => htmlentities($content),
              '#class' => $settings['class'],
              '#id' => 'file-' . $item['fid'],
              //@todo: render this field (in the future--give them the option?)
              //$item['attributes']['data-clicktoplay-content'] = drupal_render(field_view_value($entity_type, $entity, $field['field_name'], $item, array(
              //  'type' => 'file_rendered',
              //)));
            );
            // @todo: this shouldn't be necessary. Would prefer to use a render array
            $element[$delta] = array(
              '#markup' => drupal_render($element[$delta]),
            );
          }
          //if ($item);
        }
        // Gracefully degrade to a normal File Render.
        if (empty($element[$delta])) {
          // render as normal
          $element[$delta] = field_view_value($entity_type, $entity, $field['field_name'], $item, array(
            'type' => 'file_rendered',
          ));
        }
        break;      
    }

  } // foreach

  // Add a default image to image_super formats
  // @todo: this isn't working...does drupal hide a field if it is empty?
  if (empty($items) && $display['type'] === 'image_super' && !empty($settings['placeholder_dimensions'])) {
    $url = 'http://placehold.it/' . $settings['placeholder_dimensions'];
    $options = array(
      'query' => array(),
      'absolute' => TRUE
    );
    if (!empty($settings['placeholder_text'])) {
      $options['query']['text'] = $settings['placeholder_text'];
    }
    $element[0] = array(
      '#markup' => '<img src="' . url($url, $options) . '" />',
    );
    //dpm(drupal_render($element));
  }

  return $element;
}

