package org.example.registryviewer.ui;

import javax.swing.table.DefaultTableModel;
import javax.swing.JTree;

/**
 * View contract for the controller.
 */
public interface RegistryView {

    void setStatus(String message);

    JTree getKeyTree();

    DefaultTableModel getValueTableModel();
}

