package org.example.registryviewer;

import org.example.registryviewer.app.RegistryController;
import org.example.registryviewer.registry.DefaultRegistryValueDisplayFormatter;
import org.example.registryviewer.registry.JnaRegistryReadService;
import org.example.registryviewer.ui.RegistryViewerFrame;

import javax.swing.SwingUtilities;
import javax.swing.UIManager;

/**
 * Application entry: wires controller + services + Swing view.
 */
public final class Main {

    public static void main(String[] args) {
        String os = System.getProperty("os.name", "").toLowerCase();
        if (!os.contains("win")) {
            System.err.println("This application requires Microsoft Windows.");
            System.exit(1);
        }

        SwingUtilities.invokeLater(() -> {
            try {
                UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
            } catch (Exception ignored) {
                // Fall back to default L&F.
            }

            RegistryViewerFrame frame = new RegistryViewerFrame();
            RegistryController controller = new RegistryController(
                    frame,
                    new JnaRegistryReadService(),
                    new DefaultRegistryValueDisplayFormatter()
            );
            controller.initialize();

            frame.setLocationRelativeTo(null);
            frame.setVisible(true);
        });
    }

    private Main() {
    }
}

