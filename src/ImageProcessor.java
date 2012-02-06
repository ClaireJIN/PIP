package pip;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.color.ColorSpace;
import java.awt.image.BufferedImage;
import java.awt.image.ColorConvertOp;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;

/**
 *
 * @author Ovilia
 */
public class ImageProcessor {

    public static final int RANGE_OF_8BITS = 256;

    public ImageProcessor(String fileName) {
        try {
            bufferedImage = ImageIO.read(new File(fileName));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public ImageProcessor(BufferedImage bufferedImage) {
        setBufferedImage(bufferedImage);
    }

    /**
     * @return the bufferedImage
     */
    public BufferedImage getBufferedImage() {
        return bufferedImage;
    }

    /**
     * @param bufferedImage the bufferedImage to set
     */
    public void setBufferedImage(BufferedImage bufferedImage) {
        this.bufferedImage = bufferedImage;
        // other parameters will be set to recalculated
        grayScaleImage = null;
        histogram = null;
        rgbHistogram = null;
    }

    public BufferedImage getGrayScaleImage(RenderingHints hints) {
        if (grayScaleImage == null) {
            // gray scall image not calculated yet
            int width = bufferedImage.getWidth();
            int height = bufferedImage.getHeight();
            grayScaleImage = new BufferedImage(
                    width, height, bufferedImage.getType());

            if (hints == null) {
                Graphics2D g2 = grayScaleImage.createGraphics();
                hints = g2.getRenderingHints();
                g2.dispose();
                g2 = null;
            }
            ColorSpace grayCS = ColorSpace.getInstance(ColorSpace.CS_GRAY);
            ColorConvertOp colorConvertOp = new ColorConvertOp(grayCS, hints);
            colorConvertOp.filter(bufferedImage, grayScaleImage);
        }
        return grayScaleImage;
    }

    public int[] getHistogram() {
        if (histogram == null) {
            // make sure gray scale is calculated
            getGrayScaleImage(null);
            histogram = new int[RANGE_OF_8BITS];
            int width = grayScaleImage.getWidth();
            int height = grayScaleImage.getHeight();
            for (int i = 0; i < width; ++i) {
                for (int j = 0; j < height; ++j) {
                    int gray = (new Color(grayScaleImage.getRGB(i, j))).getRed();
                    histogram[gray]++;
                }
            }
        }
        return histogram;
    }

    public int[][] getRgbHistogram() {
        if (rgbHistogram == null) {
            // make sure gray scale is calculated
            getGrayScaleImage(null);
            rgbHistogram = new int[RANGE_OF_8BITS][3];
            int width = grayScaleImage.getWidth();
            int height = grayScaleImage.getHeight();
            for (int i = 0; i < width; ++i) {
                for (int j = 0; j < height; ++j) {
                    Color color = new Color(bufferedImage.getRGB(i, j));
                    rgbHistogram[color.getRed()][0]++;
                    rgbHistogram[color.getGreen()][1]++;
                    rgbHistogram[color.getBlue()][2]++;
                }
            }
        }
        return rgbHistogram;
    }

    public BufferedImage getThresholdImage(int threshold) {
        // make sure gray scale is calculated
        getGrayScaleImage(null);
        int width = grayScaleImage.getWidth();
        int height = grayScaleImage.getHeight();
        BufferedImage thresholdImage = new BufferedImage(
                width, height, grayScaleImage.getType());
        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                int gray = (new Color(grayScaleImage.getRGB(i, j))).getRed();
                Color color;
                if (gray >= threshold) {
                    color = Color.WHITE;
                } else {
                    color = Color.BLACK;
                }
                thresholdImage.setRGB(i, j, color.getRGB());
            }
        }
        return thresholdImage;
    }
    private BufferedImage bufferedImage;
    private BufferedImage grayScaleImage;
    private int[] histogram;
    private int[][] rgbHistogram;
}
