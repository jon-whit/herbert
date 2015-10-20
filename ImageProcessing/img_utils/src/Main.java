import javax.imageio.ImageIO;
import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

public class Main {

    /**
     * Main entry point for this application.
     *
     * @param args - The program arguments passed to this application.
     */
    public static void main(String[] args) {
        JFrame frame = new JFrame("Point Generator");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setLayout(new BorderLayout());

        // Add the TestPane to the main frame.
        frame.add(new TestPane(args[0]));

        frame.pack();
        frame.setLocationRelativeTo(null);
        frame.setVisible(true);
    }

    public static class TestPane extends JPanel {

        private BufferedImage img;
        private JLabel label;

        private JPanel fields;
        private JPanel outputPanel;
        private TextArea outputText;
        private JTextField red;
        private JTextField green;
        private JTextField blue;
        private JTextField xcoord;
        private JTextField ycoord;

        /**
         * Constructs a new TestPane.
         *
         * @param filepath - The file path of the source image.
         */
        public TestPane(String filepath) {

            setLayout(new GridBagLayout());
            GridBagConstraints gbc = new GridBagConstraints();
            gbc.gridwidth = GridBagConstraints.REMAINDER;


            // Attempt to read the image source file and create a JLabel component with the image.
            label = new JLabel();
            try {
                img = ImageIO.read(new File(filepath));
                label.setIcon(new ImageIcon(img));
            } catch (IOException ex) {
                ex.printStackTrace();
            }

            // Add the label to the Grid layout.
            add(label, gbc);

            fields = new JPanel();
            outputPanel = new JPanel();

            // Create border regions for the RGB fields and output panel.
            fields.setBorder(new EmptyBorder(5, 5, 5, 5));
            outputPanel.setBorder(new EmptyBorder(5, 5, 5, 5));

            // Create the RGB, x-coord, and y-coord text boxes.
            red = new JTextField(3);
            green = new JTextField(3);
            blue = new JTextField(3);
            xcoord = new JTextField(3);
            ycoord = new JTextField(3);

            outputText = new TextArea(10, 10);
            outputText.setEditable(false);

            // Add the text fields to the fields panel.
            fields.add(red);
            fields.add(green);
            fields.add(blue);
            fields.add(xcoord);
            fields.add(ycoord);

            // Add the output text field to the output panel.
            outputPanel.add(outputText);

            // Add the fields and output sub panels to this panel.
            //add(fields, gbc);
            add(outputPanel, gbc);

            label.addMouseMotionListener(new MouseAdapter() {
                @Override
                public void mouseMoved(MouseEvent e) {

                    int packedInt = img.getRGB(e.getX(), e.getY());
                    Color color = new Color(packedInt, true);
                    fields.setBackground(color);
                    red.setText(Integer.toString(color.getRed()));
                    green.setText(Integer.toString(color.getGreen()));
                    blue.setText(Integer.toString(color.getBlue()));
                    xcoord.setText(Integer.toString(e.getX()));
                    ycoord.setText(Integer.toString(e.getY()));

                }
            });

            label.addMouseListener(new MouseAdapter() {
                @Override
                public void mouseClicked(MouseEvent e) {
                    super.mouseClicked(e);

                    String x = Integer.toString(e.getX());
                    String y = Integer.toString(e.getY());

                    outputText.append(x + " " + y + "\n");
                }
            });

        }
    }
}


