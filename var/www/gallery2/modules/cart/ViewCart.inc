<?php
/*
 * $RCSfile: ViewCart.inc,v $
 *
 * Gallery - a web based photo album viewer and editor
 * Copyright (C) 2000-2006 Bharat Mediratta
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA  02110-1301, USA.
 */
/**
 * @version $Revision: 1.19 $ $Date: 2006/01/10 04:38:59 $
 * @package Cart
 * @subpackage UserInterface
 * @author Bharat Mediratta <bharat@menalto.com>
 */

/**
 * Required classes
 */
GalleryCoreApi::requireOnce('modules/cart/classes/CartHelper.class');

/**
 * This view will allow users to view their individual carts
 *
 * @package Comment
 * @subpackage UserInterface
 */
class ViewCartView extends GalleryView {

    /**
     * @see GalleryView::loadTemplate
     */
    function loadTemplate(&$template, &$form) {
	global $gallery;

	list ($ret, $itemList) = CartHelper::loadCartItems();
	if ($ret) {
	    return array($ret->wrap(__FILE__, __LINE__), null);
	}
	list ($ret, $cartItemIds) = CartHelper::fetchCartItemCounts();
	if ($ret) {
	    return array($ret->wrap(__FILE__, __LINE__), null);
	}

	if ($form['formName'] != 'ViewCart') {
	    /* First time around, initialize the cart counts */
	    foreach ($cartItemIds as $itemId => $count) {
		$form['counts'][$itemId] = $count;
	    }
	    $form['formName'] = 'ViewCart';
	}
	$ViewCart = array();

	/* Bring in the root album information for the breadcrumb */
	list ($ret, $rootAlbumId) =
	    GalleryCoreApi::getPluginParameter('module', 'core', 'id.rootAlbum');
	if ($ret) {
	    return array($ret->wrap(__FILE__, __LINE__), null);
	}
	list ($ret, $rootAlbum) = GalleryCoreApi::loadEntitiesById($rootAlbumId);
	if ($ret) {
	    return array($ret->wrap(__FILE__, __LINE__), null);
	}
	$ViewCart['rootAlbum'] = (array)$rootAlbum;

	/* Find all our cart plugins */
	GalleryCoreApi::requireOnce('modules/cart/classes/CartPluginInterface_1_0.class');
	list ($ret, $ids) =
	    GalleryCoreApi::getAllFactoryImplementationIds('CartPluginInterface_1_0');
	if ($ret) {
	    return array($ret->wrap(__FILE__, __LINE__), null);
	}
	$plugins = array();
	foreach ($ids as $id => $className) {
	    list ($ret, $instances[$id]) =
		GalleryCoreApi::newFactoryInstance('CartPluginInterface_1_0', $className);
	    if ($ret) {
		return array($ret->wrap(__FILE__, __LINE__), null);
	    }

	    /*
	     * Figure out what types each plugin can handle so that we can cue the UI
	     * to only send it the ones of the appropriate types.
	     */
	    $plugins[$id]['supportedItemTypes'] = $instances[$id]->getSupportedItemTypes();
	    list ($ret, $plugins[$id]['actionDisplayName']) =
		$instances[$id]->getActionDisplayName();
	    if ($ret) {
		return array($ret->wrap(__FILE__, __LINE__), null);
	    }
	}

	$items = array();
	$thumbnails = array();
	$itemTypeNames = array();
	if (!empty($cartItemIds)) {
	    foreach ($itemList as $item) {
		$items[$item->getId()] = (array)$item;

		/* TODO: This is a O(N^3) operation -- probably should be optimized */
		foreach ($plugins as $pluginId => $pluginData) {
		    foreach ($pluginData['supportedItemTypes'] as $type) {
			if ($type == '*') {
			    $plugins[$id]['supported'][$item->getId()] = 1;
			    break;
			} else {
			    if (GalleryUtilities::isA($item, $type)) {
				$plugins[$id]['supported'][$item->getId()] = 1;
				break;
			    }
			}
		    }
		}

		$itemTypeNames[$item->getId()] = $item->itemTypeName();
	    }

	    /* Fetch thumbnails */
	    list ($ret, $thumbnailList) =
		GalleryCoreApi::fetchThumbnailsByItemIds(array_keys($cartItemIds));
	    if ($ret) {
		return array($ret->wrap(__FILE__, __LINE__), null);
	    }
	    foreach ($thumbnailList as $thumbnail) {
		$thumbnails[$thumbnail->getParentId()] = (array)$thumbnail;
	    }
	}

	$ViewCart['items'] = $items;
	$ViewCart['itemTypeNames'] = $itemTypeNames;
	$ViewCart['thumbnails'] = $thumbnails;
	$ViewCart['plugins'] = $plugins;
	$ViewCart['controller'] = 'cart.ModifyCart';
	$template->setVariable('ViewCart', $ViewCart);

	$urlGenerator =& $gallery->getUrlGenerator();
	list ($ret, $navigationLinks) = $urlGenerator->getNavigationLinks();
	if ($ret) {
	    return array($ret->wrap(__FILE__, __LINE__), null);
	}
	$template->setVariable('navigationLinks', $navigationLinks);

	list ($ret, $module) = GalleryCoreApi::loadPlugin('module', 'cart');
	if ($ret) {
	    return array($ret->wrap(__FILE__, __LINE__), null);
	}

	$template->title($module->translate('View Your Cart'));
	return array(null,
		     array('body' => 'modules/cart/templates/ViewCart.tpl'));
    }

    /**
     * @see GalleryView::getViewDescription()
     */
    function getViewDescription() {
	list ($ret, $module) = GalleryCoreApi::loadPlugin('module', 'cart');
	if ($ret) {
	    return array($ret->wrap(__FILE__, __LINE__), null);
	}

	return array(null, $module->translate('view cart'));
    }
}
?>
