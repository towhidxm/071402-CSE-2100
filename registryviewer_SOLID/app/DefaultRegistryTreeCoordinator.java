package org.example.registryviewer.app;

import com.sun.jna.platform.win32.WinReg.HKEY;
import org.example.registryviewer.domain.RegistryKeyNode;
import org.example.registryviewer.domain.RootHive;
import org.example.registryviewer.registry.RegistryAccessException;
import org.example.registryviewer.registry.RegistryReadService;
import org.example.registryviewer.ui.RegistryTreeView;

import javax.swing.tree.DefaultMutableTreeNode;
import java.util.List;

/**
 * Tree loading logic for root hives and lazy child expansion.
 */
public final class DefaultRegistryTreeCoordinator implements RegistryTreeCoordinator {

    private final RegistryReadService registryReadService;

    /**
     * @param registryReadService abstraction for read-only registry access
     */
    public DefaultRegistryTreeCoordinator(RegistryReadService registryReadService) {
        this.registryReadService = registryReadService;
    }

    @Override
    public void populateRootKeys(RegistryTreeView view) {
        DefaultMutableTreeNode root = view.getInvisibleTreeRoot();
        root.removeAllChildren();

        for (RootHive hive : RootHive.values()) {
            RegistryKeyNode nodeData = RegistryKeyNode.realKey(hive.getDisplayName(), hive.getDisplayName(), hive.getHkey());
            DefaultMutableTreeNode node = new DefaultMutableTreeNode(nodeData);
            node.add(new DefaultMutableTreeNode(RegistryKeyNode.placeholderChild()));
            root.add(node);
        }

        view.getTreeModel().reload();
    }

    @Override
    public void expandNode(RegistryTreeView view, DefaultMutableTreeNode node, RegistryKeyNode data) {
        HKEY parentHkey = data.getHkey();
        try {
            List<String> children = registryReadService.listSubkeyNames(parentHkey);
            for (String childName : children) {
                try {
                    HKEY childHkey = registryReadService.openChildReadOnly(parentHkey, childName);
                    String fullPath = data.getFullPath() + "\\" + childName;
                    DefaultMutableTreeNode child = new DefaultMutableTreeNode(
                            RegistryKeyNode.realKey(childName, fullPath, childHkey));
                    child.add(new DefaultMutableTreeNode(RegistryKeyNode.placeholderChild()));
                    node.add(child);
                } catch (RegistryAccessException ignored) {
                    // Keep behavior: silently skip keys we cannot open.
                }
            }
        } catch (RegistryAccessException ex) {
            view.setStatus("Error listing subkeys: " + ex.getMessage());
        }

        view.getTreeModel().reload(node);
    }
}
