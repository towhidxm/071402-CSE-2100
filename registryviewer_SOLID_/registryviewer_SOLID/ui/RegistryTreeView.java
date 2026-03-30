package org.example.registryviewer.ui;

import javax.swing.JToggleButton;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;

/**
 * View contract that includes tree + auto-refresh control.
 */
public interface RegistryTreeView extends RegistryView {

    DefaultTreeModel getTreeModel();

    DefaultMutableTreeNode getInvisibleTreeRoot();

    JToggleButton getAutoRefreshToggle();
}

