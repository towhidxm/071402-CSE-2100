package org.example.registryviewer.app;

import com.sun.jna.platform.win32.WinReg.HKEY;
import org.example.registryviewer.config.AppConstants;
import org.example.registryviewer.domain.RegistryKeyNode;
import org.example.registryviewer.model.RegistrySessionModel;
import org.example.registryviewer.registry.RegistryReadService;
import org.example.registryviewer.registry.RegistryValueDisplayFormatter;
import org.example.registryviewer.ui.RegistryTreeView;

import javax.swing.Timer;
import javax.swing.event.TreeExpansionEvent;
import javax.swing.event.TreeExpansionListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreePath;
import java.awt.event.ItemEvent;
import java.util.concurrent.TimeUnit;

/**
 * Coordinates user actions with the registry service and view (MVC controller).
 */
public final class RegistryController implements TreeSelectionListener, TreeExpansionListener {

    private final RegistryTreeView view;
    private final RegistryTreeCoordinator treeCoordinator;
    private final RegistryValueCoordinator valueCoordinator;

    private final RegistrySessionModel sessionModel;
    private final Timer refreshTimer;

    public RegistryController(
            RegistryTreeView view,
            RegistryReadService registryReadService,
            RegistryValueDisplayFormatter valueFormatter) {
        this.view = view;
        this.treeCoordinator = new DefaultRegistryTreeCoordinator(registryReadService);
        this.valueCoordinator = new DefaultRegistryValueCoordinator(registryReadService, valueFormatter);
        this.sessionModel = new RegistrySessionModel();

        this.refreshTimer = new Timer(
                (int) TimeUnit.SECONDS.toMillis(AppConstants.AUTO_REFRESH_INTERVAL_SEC),
                e -> loadValuesForCurrentKey()
        );
    }

    public void initialize() {
        treeCoordinator.populateRootKeys(view);

        view.getKeyTree().addTreeSelectionListener(this);
        view.getKeyTree().addTreeExpansionListener(this);
        view.getAutoRefreshToggle().addItemListener(this::onAutoRefreshToggle);

        view.setStatus("Ready  –  Select a key to begin.");
    }

    private void onAutoRefreshToggle(ItemEvent e) {
        if (e.getStateChange() == ItemEvent.SELECTED) {
            sessionModel.setAutoRefreshEnabled(true);
            refreshTimer.start();
            view.setStatus("Auto-refresh ON");
        } else {
            sessionModel.setAutoRefreshEnabled(false);
            refreshTimer.stop();
            view.setStatus("Auto-refresh OFF");
        }
    }

    @Override
    public void valueChanged(TreeSelectionEvent e) {
        TreePath path = view.getKeyTree().getSelectionPath();
        if (path == null) {
            return;
        }

        DefaultMutableTreeNode treeNode = (DefaultMutableTreeNode) path.getLastPathComponent();
        Object uo = treeNode.getUserObject();
        if (!(uo instanceof RegistryKeyNode keyNode)) {
            return;
        }
        if (keyNode.isPlaceholder() || keyNode.getHkey() == null) {
            sessionModel.clearSelection();
            return;
        }

        sessionModel.selectKey(keyNode.getHkey(), keyNode.getFullPath());
        view.setStatus("Viewing: " + sessionModel.getSelectedPath());
        loadValuesForCurrentKey();
    }

    @Override
    public void treeExpanded(TreeExpansionEvent event) {
        DefaultMutableTreeNode node = (DefaultMutableTreeNode) event.getPath().getLastPathComponent();
        Object uo = node.getUserObject();
        if (!(uo instanceof RegistryKeyNode data) || data.isPlaceholder()) {
            return;
        }

        // Only populate if the node still has a single "Loading..." placeholder child.
        if (node.getChildCount() != 1) {
            return;
        }

        DefaultMutableTreeNode first = (DefaultMutableTreeNode) node.getFirstChild();
        Object firstUo = first.getUserObject();
        if (!(firstUo instanceof RegistryKeyNode firstData) || !firstData.isLoadingPlaceholder()) {
            return;
        }

        node.removeAllChildren();

        treeCoordinator.expandNode(view, node, data);
    }

    @Override
    public void treeCollapsed(TreeExpansionEvent event) {
        // no-op
    }

    private void loadValuesForCurrentKey() {
        HKEY selectedHkey = sessionModel.getSelectedHkey();
        if (selectedHkey == null) {
            return;
        }
        valueCoordinator.loadValuesForKey(view, selectedHkey);
    }
}

