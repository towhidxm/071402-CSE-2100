package org.example.registryviewer.ui;

import org.example.registryviewer.config.AppConstants;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableRowSorter;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreePath;
import java.awt.*;
import java.awt.datatransfer.StringSelection;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

/**
 * Enhanced Swing layout: search/filter, dark mode, bookmarks, copy, export CSV.
 */
public final class RegistryViewerFrame extends JFrame implements RegistryTreeView {

    private final JTree keyTree;
    private final DefaultTableModel valueTableModel;
    private final JLabel statusLabel;
    private final JToggleButton autoRefreshToggle;
    private final DefaultTreeModel treeModel;
    private final DefaultMutableTreeNode invisibleRoot;

    private final JTextField searchField;
    private final List<String> bookmarks = new ArrayList<>();
    private final DefaultListModel<String> bookmarkListModel = new DefaultListModel<>();
    private final JList<String> bookmarkList;
    private TableRowSorter<DefaultTableModel> tableSorter;
    private JTable valueTable;

    private boolean darkMode = false;
    private static final Color DARK_BG    = new Color(30, 30, 30);
    private static final Color DARK_FG    = new Color(220, 220, 220);
    private static final Color DARK_PANEL = new Color(45, 45, 45);
    private static final Color DARK_SEL   = new Color(70, 130, 180);

    private JPanel contentPanel;
    private JScrollPane treeScroll, tableScroll, bookmarkScroll;
    private JPanel searchPanel, statusPanel, bookmarkPanel;
    private JToolBar toolBar;

    public RegistryViewerFrame() {
        super(AppConstants.APP_TITLE + " — Enhanced Edition");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setPreferredSize(new Dimension(1300, 820));
        setMinimumSize(new Dimension(900, 600));

        invisibleRoot = new DefaultMutableTreeNode();
        treeModel = new DefaultTreeModel(invisibleRoot);
        keyTree = new JTree(treeModel);
        keyTree.setRootVisible(false);
        keyTree.setShowsRootHandles(true);
        keyTree.setFont(new Font("Segoe UI", Font.PLAIN, 13));

        valueTableModel = new DefaultTableModel(new Object[]{"Name", "Type", "Data"}, 0) {
            @Override public boolean isCellEditable(int r, int c) { return false; }
        };
        valueTable = new JTable(valueTableModel);
        valueTable.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
        valueTable.setRowHeight(22);
        valueTable.setFont(new Font("Segoe UI", Font.PLAIN, 13));
        valueTable.getTableHeader().setFont(new Font("Segoe UI", Font.BOLD, 13));
        valueTable.getColumnModel().getColumn(0).setPreferredWidth(160);
        valueTable.getColumnModel().getColumn(1).setPreferredWidth(140);
        valueTable.getColumnModel().getColumn(2).setPreferredWidth(400);
        valueTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        tableSorter = new TableRowSorter<>(valueTableModel);
        valueTable.setRowSorter(tableSorter);

        // Table right-click menu
        JPopupMenu tablePopup = new JPopupMenu();
        JMenuItem copyName = new JMenuItem("Copy Name");
        JMenuItem copyData = new JMenuItem("Copy Data");
        JMenuItem copyRow  = new JMenuItem("Copy Full Row");
        tablePopup.add(copyName); tablePopup.add(copyData); tablePopup.addSeparator(); tablePopup.add(copyRow);
        valueTable.setComponentPopupMenu(tablePopup);
        copyName.addActionListener(e -> copyCell(0));
        copyData.addActionListener(e -> copyCell(2));
        copyRow.addActionListener(e -> copyRow());

        // Tree right-click menu
        JPopupMenu treePopup = new JPopupMenu();
        JMenuItem addBookmark = new JMenuItem("Add to Bookmarks");
        JMenuItem copyPath    = new JMenuItem("Copy Key Path");
        treePopup.add(addBookmark); treePopup.add(copyPath);
        keyTree.setComponentPopupMenu(treePopup);
        addBookmark.addActionListener(e -> bookmarkSelectedKey());
        copyPath.addActionListener(e -> copySelectedKeyPath());

        // Search/filter bar
        searchField = new JTextField();
        searchField.setFont(new Font("Segoe UI", Font.PLAIN, 13));
        JButton clearBtn = new JButton("x");
        clearBtn.setFocusPainted(false);
        clearBtn.addActionListener(e -> { searchField.setText(""); applyFilter(""); });
        searchField.getDocument().addDocumentListener(new javax.swing.event.DocumentListener() {
            public void insertUpdate(javax.swing.event.DocumentEvent e)  { applyFilter(searchField.getText()); }
            public void removeUpdate(javax.swing.event.DocumentEvent e)  { applyFilter(searchField.getText()); }
            public void changedUpdate(javax.swing.event.DocumentEvent e) { applyFilter(searchField.getText()); }
        });

        searchPanel = new JPanel(new BorderLayout(4, 0));
        searchPanel.setBorder(new EmptyBorder(4, 6, 4, 6));
        JLabel lbl = new JLabel("Filter: ");
        lbl.setFont(new Font("Segoe UI", Font.BOLD, 13));
        searchPanel.add(lbl, BorderLayout.WEST);
        searchPanel.add(searchField, BorderLayout.CENTER);
        searchPanel.add(clearBtn, BorderLayout.EAST);

        // Bookmark panel
        bookmarkList = new JList<>(bookmarkListModel);
        bookmarkList.setFont(new Font("Segoe UI", Font.PLAIN, 12));
        bookmarkScroll = new JScrollPane(bookmarkList);
        JButton removeBm = new JButton("Remove Selected");
        removeBm.setFont(new Font("Segoe UI", Font.PLAIN, 12));
        removeBm.addActionListener(e -> removeBookmark());
        bookmarkPanel = new JPanel(new BorderLayout());
        bookmarkPanel.setBorder(BorderFactory.createTitledBorder("Bookmarks"));
        bookmarkPanel.add(bookmarkScroll, BorderLayout.CENTER);
        bookmarkPanel.add(removeBm, BorderLayout.SOUTH);
        bookmarkPanel.setPreferredSize(new Dimension(0, 150));

        // Toolbar
        autoRefreshToggle = new JToggleButton("Auto-refresh");
        autoRefreshToggle.setToolTipText("Refresh every " + AppConstants.AUTO_REFRESH_INTERVAL_SEC + "s");

        JToggleButton darkBtn = new JToggleButton("Dark Mode");
        darkBtn.addActionListener(e -> toggleDark(darkBtn.isSelected()));

        JButton exportBtn = new JButton("Export CSV");
        exportBtn.addActionListener(e -> exportCSV());

        JButton expandBtn   = new JButton("Expand All");
        JButton collapseBtn = new JButton("Collapse All");
        expandBtn.addActionListener(e -> { for (int i=0;i<keyTree.getRowCount();i++) keyTree.expandRow(i); });
        collapseBtn.addActionListener(e -> { for (int i=keyTree.getRowCount()-1;i>=0;i--) keyTree.collapseRow(i); });

        toolBar = new JToolBar();
        toolBar.setFloatable(false);
        toolBar.setBorder(new EmptyBorder(4, 8, 4, 8));
        JLabel title = new JLabel("Registry Viewer  ");
        title.setFont(new Font("Segoe UI", Font.BOLD, 15));
        toolBar.add(title);
        toolBar.addSeparator();
        toolBar.add(new JLabel("READ-ONLY"));
        toolBar.addSeparator();
        toolBar.add(expandBtn);
        toolBar.add(collapseBtn);
        toolBar.add(Box.createHorizontalGlue());
        toolBar.add(exportBtn);
        toolBar.addSeparator();
        toolBar.add(autoRefreshToggle);
        toolBar.addSeparator();
        toolBar.add(darkBtn);

        // Status bar
        statusLabel = new JLabel("  Ready");
        statusLabel.setFont(new Font("Segoe UI", Font.PLAIN, 12));
        statusLabel.setBorder(BorderFactory.createLoweredBevelBorder());
        statusPanel = new JPanel(new BorderLayout());
        statusPanel.add(statusLabel, BorderLayout.CENTER);

        // Layout assembly
        treeScroll  = new JScrollPane(keyTree);
        tableScroll = new JScrollPane(valueTable);

        JPanel rightPanel = new JPanel(new BorderLayout());
        rightPanel.add(searchPanel, BorderLayout.NORTH);
        rightPanel.add(tableScroll, BorderLayout.CENTER);

        JSplitPane rightSplit = new JSplitPane(JSplitPane.VERTICAL_SPLIT, rightPanel, bookmarkPanel);
        rightSplit.setResizeWeight(0.80);
        rightSplit.setDividerLocation(600);

        JSplitPane mainSplit = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, treeScroll, rightSplit);
        mainSplit.setDividerLocation(320);
        mainSplit.setResizeWeight(0.25);

        contentPanel = new JPanel(new BorderLayout());
        contentPanel.add(toolBar, BorderLayout.NORTH);
        contentPanel.add(mainSplit, BorderLayout.CENTER);
        contentPanel.add(statusPanel, BorderLayout.SOUTH);
        setContentPane(contentPanel);
        pack();
        setLocationRelativeTo(null);
    }

    private void applyFilter(String text) {
        if (text == null || text.isEmpty()) {
            tableSorter.setRowFilter(null);
        } else {
            try { tableSorter.setRowFilter(RowFilter.regexFilter("(?i)" + text, 0, 2)); }
            catch (Exception ignored) { tableSorter.setRowFilter(null); }
        }
    }

    private void copyCell(int col) {
        int row = valueTable.getSelectedRow();
        if (row < 0) return;
        Object v = valueTable.getValueAt(row, col);
        Toolkit.getDefaultToolkit().getSystemClipboard().setContents(new StringSelection(v == null ? "" : v.toString()), null);
        setStatus("Copied to clipboard.");
    }

    private void copyRow() {
        int row = valueTable.getSelectedRow();
        if (row < 0) return;
        StringBuilder sb = new StringBuilder();
        for (int c = 0; c < 3; c++) { if (c>0) sb.append("\t"); Object v = valueTable.getValueAt(row,c); sb.append(v==null?"":v); }
        Toolkit.getDefaultToolkit().getSystemClipboard().setContents(new StringSelection(sb.toString()), null);
        setStatus("Row copied.");
    }

    private void copySelectedKeyPath() {
        TreePath path = keyTree.getSelectionPath();
        if (path == null) return;
        Object uo = ((DefaultMutableTreeNode) path.getLastPathComponent()).getUserObject();
        Toolkit.getDefaultToolkit().getSystemClipboard().setContents(new StringSelection(uo.toString()), null);
        setStatus("Key path copied.");
    }

    private void bookmarkSelectedKey() {
        TreePath path = keyTree.getSelectionPath();
        if (path == null) return;
        String label = ((DefaultMutableTreeNode) path.getLastPathComponent()).getUserObject().toString();
        if (!bookmarks.contains(label)) { bookmarks.add(label); bookmarkListModel.addElement(label); setStatus("Bookmarked: " + label); }
        else setStatus("Already bookmarked.");
    }

    private void removeBookmark() {
        int idx = bookmarkList.getSelectedIndex();
        if (idx >= 0) { bookmarks.remove(idx); bookmarkListModel.remove(idx); }
    }

    private void exportCSV() {
        if (valueTableModel.getRowCount() == 0) {
            JOptionPane.showMessageDialog(this, "No data to export.", "Export", JOptionPane.INFORMATION_MESSAGE); return;
        }
        JFileChooser fc = new JFileChooser();
        fc.setSelectedFile(new File("registry_export.csv"));
        if (fc.showSaveDialog(this) != JFileChooser.APPROVE_OPTION) return;
        try (PrintWriter pw = new PrintWriter(new FileWriter(fc.getSelectedFile()))) {
            pw.println("\"Name\",\"Type\",\"Data\"");
            for (int r = 0; r < valueTableModel.getRowCount(); r++) {
                StringBuilder sb = new StringBuilder();
                for (int c = 0; c < 3; c++) {
                    if (c>0) sb.append(",");
                    Object v = valueTableModel.getValueAt(r, c);
                    sb.append("\"").append(v == null ? "" : v.toString().replace("\"","\"\"")).append("\"");
                }
                pw.println(sb);
            }
            setStatus("Exported: " + fc.getSelectedFile().getName());
            JOptionPane.showMessageDialog(this, "Exported successfully!", "Export CSV", JOptionPane.INFORMATION_MESSAGE);
        } catch (IOException ex) {
            JOptionPane.showMessageDialog(this, "Export failed: " + ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
        }
    }

    private void toggleDark(boolean dark) {
        this.darkMode = dark;
        Color bg  = dark ? DARK_BG    : Color.WHITE;
        Color fg  = dark ? DARK_FG    : Color.BLACK;
        Color pan = dark ? DARK_PANEL : UIManager.getColor("Panel.background");
        Color sel = dark ? DARK_SEL   : UIManager.getColor("List.selectionBackground");

        keyTree.setBackground(bg); keyTree.setForeground(fg);
        treeScroll.getViewport().setBackground(bg);
        valueTable.setBackground(bg); valueTable.setForeground(fg);
        valueTable.getTableHeader().setBackground(dark ? new Color(55,55,55) : null);
        valueTable.getTableHeader().setForeground(fg);
        valueTable.setSelectionBackground(sel);
        tableScroll.getViewport().setBackground(bg);
        searchField.setBackground(dark ? new Color(55,55,55) : Color.WHITE);
        searchField.setForeground(fg); searchField.setCaretColor(fg);
        searchPanel.setBackground(pan);
        statusLabel.setForeground(fg); statusPanel.setBackground(pan);
        toolBar.setBackground(pan); contentPanel.setBackground(pan);
        bookmarkPanel.setBackground(pan);
        bookmarkList.setBackground(bg); bookmarkList.setForeground(fg);
        bookmarkScroll.getViewport().setBackground(bg);
        repaint();
        setStatus(dark ? "Dark mode ON" : "Light mode ON");
    }

    @Override public void setStatus(String message) { statusLabel.setText(message == null ? " " : "  " + message); }
    @Override public JTree getKeyTree() { return keyTree; }
    @Override public DefaultTableModel getValueTableModel() { return valueTableModel; }
    @Override public DefaultTreeModel getTreeModel() { return treeModel; }
    @Override public DefaultMutableTreeNode getInvisibleTreeRoot() { return invisibleRoot; }
    @Override public JToggleButton getAutoRefreshToggle() { return autoRefreshToggle; }
}
