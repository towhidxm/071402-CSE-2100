package org.example.registryviewer.ui;

import org.example.registryviewer.config.AppConstants;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTable;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.JTree;
import javax.swing.SwingConstants;
import javax.swing.table.DefaultTableModel;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import java.awt.BorderLayout;
import java.awt.Dimension;

/**
 * Swing layout only (no registry calls).
 */
public final class RegistryViewerFrame extends JFrame implements RegistryTreeView {

    private final JTree keyTree;
    private final DefaultTableModel valueTableModel;
    private final JLabel statusLabel;
    private final JToggleButton autoRefreshToggle;

    private final DefaultTreeModel treeModel;
    private final DefaultMutableTreeNode invisibleRoot;

    public RegistryViewerFrame() {
        super(AppConstants.APP_TITLE);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setPreferredSize(new Dimension(1100, 750));

        invisibleRoot = new DefaultMutableTreeNode();
        treeModel = new DefaultTreeModel(invisibleRoot);
        keyTree = new JTree(treeModel);
        keyTree.setRootVisible(false);
        keyTree.setShowsRootHandles(true);

        valueTableModel = new DefaultTableModel(new Object[]{"Name", "Type", "Data"}, 0) {
            @Override
            public boolean isCellEditable(int row, int column) {
                return false;
            }
        };

        JTable valueTable = new JTable(valueTableModel);
        valueTable.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
        valueTable.getColumnModel().getColumn(0).setPreferredWidth(150);
        valueTable.getColumnModel().getColumn(1).setPreferredWidth(130);
        valueTable.getColumnModel().getColumn(2).setPreferredWidth(300);

        JScrollPane left = new JScrollPane(keyTree);
        JScrollPane right = new JScrollPane(valueTable);
        JSplitPane split = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, left, right);
        split.setDividerLocation(310);
        split.setResizeWeight(0.25);

        statusLabel = new JLabel(" ");
        statusLabel.setBorder(BorderFactory.createLoweredBevelBorder());
        statusLabel.setHorizontalAlignment(SwingConstants.LEFT);

        autoRefreshToggle = new JToggleButton("Auto-refresh");
        autoRefreshToggle.setToolTipText("Refresh the value pane every " + AppConstants.AUTO_REFRESH_INTERVAL_SEC + " seconds");

        JToolBar toolBar = new JToolBar();
        toolBar.setFloatable(false);
        toolBar.add(new JLabel("READ-ONLY MODE"));
        toolBar.addSeparator();
        toolBar.add(new JLabel("Select a key to view its values."));
        toolBar.add(Box.createHorizontalGlue());
        toolBar.add(autoRefreshToggle);

        JPanel content = new JPanel(new BorderLayout());
        content.add(toolBar, BorderLayout.NORTH);
        content.add(split, BorderLayout.CENTER);
        content.add(statusLabel, BorderLayout.SOUTH);

        setContentPane(content);
        pack();
    }

    @Override
    public void setStatus(String message) {
        statusLabel.setText(message == null ? "" : message);
    }

    @Override
    public JTree getKeyTree() {
        return keyTree;
    }

    @Override
    public DefaultTableModel getValueTableModel() {
        return valueTableModel;
    }

    @Override
    public DefaultTreeModel getTreeModel() {
        return treeModel;
    }

    @Override
    public DefaultMutableTreeNode getInvisibleTreeRoot() {
        return invisibleRoot;
    }

    @Override
    public JToggleButton getAutoRefreshToggle() {
        return autoRefreshToggle;
    }
}

