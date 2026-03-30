package org.example.registryviewer.app;

import org.example.registryviewer.domain.RegistryKeyNode;
import org.example.registryviewer.ui.RegistryTreeView;

import javax.swing.tree.DefaultMutableTreeNode;

/**
 * Handles tree-only behavior so the controller stays focused on event orchestration.
 */
public interface RegistryTreeCoordinator {

    /**
     * Loads root hives into the tree model.
     *
     * @param view active tree-aware view
     */
    void populateRootKeys(RegistryTreeView view);

    /**
     * Lazy-loads children for an expanded node.
     *
     * @param view active tree-aware view
     * @param node expanded Swing tree node
     * @param data metadata of the expanded registry key
     */
    void expandNode(RegistryTreeView view, DefaultMutableTreeNode node, RegistryKeyNode data);
}
