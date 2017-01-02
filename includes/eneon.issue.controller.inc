<?php
/*
  * Copyright (C) 2016
  *
  * This program is free software; you can redistribute it and/or modify it
  * under the terms of the GNU General Public License version 2 as published
  * by the Free Software Foundation.
  *
  * If the program is linked with libraries which are licensed under one of
  * the following licenses, the combination of the program with the linked
  * library is not considered a "derivative work" of the program:
  *
  *     - Apache License, version 2.0
  *     - Apache Software License, version 1.0
  *     - GNU Lesser General Public License, version 3
  *     - Mozilla Public License, versions 1.0, 1.1 and 2.0
  *     - Common Development and Distribution License (CDDL), version 1.0
  *
  * Therefore the distribution of the program linked with libraries licensed
  * under the aforementioned licenses, is permitted by the copyright holders
  * if the distribution is compliant with both the GNU General Public
  * License version 2 and the aforementioned licenses.
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
  * Public License for more details.
  */


class IssueEntity extends Entity {

  protected function defaultLabel() {
    if (!isset($this->title)) {
      return '';
    }
    return $this->title;
  }

  protected function defaultUri() {
    return array('path' => ENEON_PATH_ISSUES . '/' . $this->identifier());
  }
}


class IssueEntityController extends EntityAPIController {

  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {

    $content = parent::buildContent($entity, $view_mode, $langcode, $content);
    $wrapper = entity_metadata_wrapper(ENEON_ENTITY_ISSUE, $entity);

    $content['description'] = array(
        '#type' => 'markup',
        '#markup' => check_plain($entity->description),
        '#prefix' => '<div class="issue-description">',
        '#suffix' => '</div>',
    );

    return $content;
  }

}
