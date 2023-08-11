#define QT_NO_KEYWORDS  // Define QT_NO_KEYWORDS before including any Qt headers
const int CUTOFF = 100;

#include "ImageColorChange.h"

#include <QtWidgets>
#include <QtGui>
#include <QtCore/QElapsedTimer>
#include <QtCore/QThreadPool>
#include <QtCore/QString>
#include <QtCore/QObject>

#include <functional>
#include <algorithm>
#include <tbb/task_group.h>

struct ColorCount {
    QColor color;
    int count;
};

namespace std
{
    template <>
    struct hash<QColor>
    {
        std::size_t operator()(const QColor& color) const
        {
            return qHash(color.rgb());
        }
    };
}

ImageColorChange::ImageColorChange(QWidget* parent)
    : QMainWindow(parent)
{
    // Create widgets
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);


    imageLabel = new QLabel("No Image Selected", this);
    imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(imageLabel);

    importButton = new QPushButton("Import Picture", this);
    layout->addWidget(importButton);

    QHBoxLayout* colorLayout = new QHBoxLayout;
    colorLabel1 = createColorLabel();
    colorLabel2 = createColorLabel();
    colorLabel3 = createColorLabel();
    colorLayout->addWidget(colorLabel1);
    colorLayout->addWidget(colorLabel2);
    colorLayout->addWidget(colorLabel3);
    layout->addLayout(colorLayout);


    QHBoxLayout* pickerButtonLayout = new QHBoxLayout;
    pickerButton1 = new QPushButton("Pick Color 1", this);

    pickerButton2 = new QPushButton("Pick Color 2", this);
    pickerButton2->setEnabled(false);

    pickerButtonLayout->addWidget(pickerButton1);
    pickerButtonLayout->addWidget(pickerButton2);

    layout->addLayout(pickerButtonLayout);


    changeColorButton = new QPushButton("Change Color", this);
    changeColorButton->setEnabled(false);
    layout->addWidget(changeColorButton);

    filterButton = new QPushButton("Apply Blur Filter", this);
    filterButton->setEnabled(false);
    layout->addWidget(filterButton);


    QHBoxLayout* timeLayout = new QHBoxLayout;
    timeLabel1 = createTimeLabel();
    timeLabel2 = createTimeLabel();
    timeLabel3 = createTimeLabel();
    timeLayout->addWidget(timeLabel1);
    timeLayout->addWidget(timeLabel2);
    timeLayout->addWidget(timeLabel3);
    layout->addLayout(timeLayout);

    QHBoxLayout* timeLayoutParallel = new QHBoxLayout;
    timeLabelParallel1 = createTimeLabel();
    timeLabelParallel2 = createTimeLabel();
    timeLabelParallel3 = createTimeLabel();
    timeLayoutParallel->addWidget(timeLabelParallel1);
    timeLayoutParallel->addWidget(timeLabelParallel2);
    timeLayoutParallel->addWidget(timeLabelParallel3);
    layout->addLayout(timeLayoutParallel);

    // Connect signals and slots
    connect(importButton, &QPushButton::clicked, this, &ImageColorChange::openImageDialog);
    connect(pickerButton1, &QPushButton::clicked, this, &ImageColorChange::pickColor1);
    connect(pickerButton2, &QPushButton::clicked, this, &ImageColorChange::pickColor2);

    connect(changeColorButton, &QPushButton::clicked, this, &ImageColorChange::colorChangingGUI);

    connect(filterButton, &QPushButton::clicked, this, &ImageColorChange::applyFilterGUI);

    setCentralWidget(centralWidget);

    // Set the fixed window size
    setFixedSize(600, 400);
    setWindowTitle("Image Color Change"); // Set the window title
}

void ImageColorChange::openImageDialog()
{
    QString imagePath = QFileDialog::getOpenFileName(this, "Select Image");
    if (!imagePath.isEmpty()) {
        QImage image(imagePath);
        imageLabel->setPixmap(QPixmap::fromImage(image).scaled(300, 300, Qt::KeepAspectRatio));

        pickerButton1->setEnabled(true);

        colorLabel1->setText(QString("RGB: 255, 255, 255"));
        colorLabel1->setStyleSheet("background-color: white; color: black;");
        colorLabel2->setText(QString("RGB: 255, 255, 255"));
        colorLabel2->setStyleSheet("background-color: white; color: black;");
        colorLabel3->setText(QString("RGB: 255, 255, 255"));
        colorLabel3->setStyleSheet("background-color: white; color: black;");


        changeColorButton->setEnabled(true);
        filterButton->setEnabled(true);

        timeLabel1->setText(QString("Calculating time.."));
        timeLabel2->setText(QString("Calculating time.."));
        timeLabel3->setText(QString("Calculating time.."));
        timeLabelParallel1->setText(QString("Calculating time.."));
        timeLabelParallel2->setText(QString("Calculating time.."));
        timeLabelParallel3->setText(QString("Calculating time.."));
        
        mostCommonColorsGUI();
    }
}

void ImageColorChange::pickColor1()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Pick Color 1");
    if (color.isValid()) {
        pickerButton1->setStyleSheet(QString("background-color: %1").arg(color.name()));
        pickerButton1->setText(QString("RGB: %1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue()));
        pickerButton2->setEnabled(true);
    }
}

void ImageColorChange::pickColor2()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Pick Color 2");
    if (color.isValid()) {
        pickerButton2->setStyleSheet(QString("background-color: %1").arg(color.name()));
        pickerButton2->setText(QString("RGB: %1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue()));
    }
}

QLabel* ImageColorChange::createColorLabel()
{
    QLabel* colorLabel = new QLabel("RGB: 255, 255, 255", this);
    colorLabel->setStyleSheet("background-color: white; color: black;");
    colorLabel->setAlignment(Qt::AlignCenter);
    return colorLabel;
}

QLabel* ImageColorChange::createTimeLabel()
{
    QLabel* timeLabel = new QLabel("Calculating Time..", this);
    timeLabel->setStyleSheet("background-color: white; color: black;");
    timeLabel->setAlignment(Qt::AlignCenter);
    return timeLabel;
}



void ImageColorChange::mostCommonColorsGUI()
{
    QImage image = imageLabel->pixmap().toImage();

    //calculating time for serial function
    QElapsedTimer timer;
    timer.start();

    QVector<QColor> sortedColors = findMostCommonColors(image);

    qint64 elapsedTime = timer.elapsed();
    timeLabel1->setText(QString("Most Common Serial: %1 s").arg(QString::number(elapsedTime / 1000.0, 'f', 4)));


    //calculating time for parallel function
    QElapsedTimer timerParallel;
    timerParallel.start();

    QVector<QColor> sortedColorsParallel = findMostCommonColorsParallel(image);

    qint64 elapsedTimeParallel = timerParallel.elapsed();
    timeLabelParallel1->setText(QString("Most Common Parallel: %1 s").arg(QString::number(elapsedTimeParallel / 1000.0, 'f', 4)));


    // Set the background color and text for colorLabel1
    if (!sortedColors.isEmpty()) {
        QColor color1 = sortedColors[0];
        colorLabel1->setStyleSheet(QString("background-color: %1").arg(color1.name()));
        colorLabel1->setText(QString("RGB: %1, %2, %3").arg(color1.red()).arg(color1.green()).arg(color1.blue()));
    }

    // Set the background color and text for colorLabel2
    if (sortedColors.size() > 1) {
        QColor color2 = sortedColors[1];
        colorLabel2->setStyleSheet(QString("background-color: %1").arg(color2.name()));
        colorLabel2->setText(QString("RGB: %1, %2, %3").arg(color2.red()).arg(color2.green()).arg(color2.blue()));
    }

    // Set the background color and text for colorLabel3
    if (sortedColors.size() > 2) {
        QColor color3 = sortedColors[2];
        colorLabel3->setStyleSheet(QString("background-color: %1").arg(color3.name()));
        colorLabel3->setText(QString("RGB: %1, %2, %3").arg(color3.red()).arg(color3.green()).arg(color3.blue()));
    }
}

void ImageColorChange::colorChangingGUI()
{
    QImage image = imageLabel->pixmap().toImage();

    QColor targetColor = pickerButton1->palette().button().color();
    QColor replacementColor = pickerButton2->palette().button().color();

    const int colorTolerance = 50; // Define a tolerance value for color similarity


    //calculating time for serial function
    QElapsedTimer timer;
    timer.start();

    replaceColor(image, targetColor, replacementColor, colorTolerance);

    qint64 elapsedTime = timer.elapsed();
    timeLabel2->setText(QString("Changing Colors Serial: %1 s").arg(QString::number(elapsedTime / 1000.0, 'f', 4)));


    //calculating time for parallel function
    QElapsedTimer timerParallel;
    timerParallel.start();

    replaceColorParallel(image, targetColor, replacementColor, colorTolerance);

    qint64 elapsedTimeParallel = timerParallel.elapsed();
    timeLabelParallel2->setText(QString("Changing Colors Parallel: %1 s").arg(QString::number(elapsedTimeParallel / 1000.0, 'f', 4)));


    // Update the image label with the modified image
    imageLabel->setPixmap(QPixmap::fromImage(image).scaled(300, 300, Qt::KeepAspectRatio));

    mostCommonColorsGUI();
}

void ImageColorChange::applyFilterGUI()
{
    QImage sourceImage = imageLabel->pixmap().toImage();

    //calculating time for parallel function
    QElapsedTimer timer;
    timer.start();

    applyFilter(sourceImage);

    qint64 elapsedTime = timer.elapsed();
    timeLabel3->setText(QString("Filter Serial: %1 s").arg(QString::number(elapsedTime / 1000.0, 'f', 4)));


    //calculating time for parallel function
    QElapsedTimer timerParallel;
    timerParallel.start();

    applyFilterParallel(sourceImage);

    qint64 elapsedTimeParallel = timerParallel.elapsed();
    timeLabelParallel3->setText(QString("Filter Parallel: %1 s").arg(QString::number(elapsedTimeParallel / 1000.0, 'f', 4)));



    imageLabel->setPixmap(QPixmap::fromImage(sourceImage).scaled(300, 300, Qt::KeepAspectRatio));
}


//serial function for find most common colors
QVector<QColor> ImageColorChange::findMostCommonColors(const QImage& image) {
    QHash<QColor, int> colorCounts;

    // Iterate through each pixel of the image and count colors
    for (int i = 0; i < image.height(); ++i) {
        for (int j = 0; j < image.width(); ++j) {
            QColor color = image.pixelColor(j, i);
            colorCounts[color]++;
        }
    }

    // Sort the colors based on their occurrence count
    QList<QColor> sortedColors = colorCounts.keys();
    std::sort(sortedColors.begin(), sortedColors.end(), [&](const QColor& color1, const QColor& color2) {
        return colorCounts[color1] > colorCounts[color2];
        });

    // Select the top 3 most common colors
    QVector<QColor> mostCommonColors;
    for (int i = 0; i < 3 && i < sortedColors.size(); ++i) {
        mostCommonColors.push_back(sortedColors[i]);
    }

    return mostCommonColors;
}

//parallel function for find most common colors
QVector<QColor> ImageColorChange::findMostCommonColorsParallel(const QImage& image) {
    QVector<QColor> mostCommonColors;
    QVector<QVector<QColor>> partialResults(4); // Vector to store partial results for each image part
    QHash<QColor, int> colorCounts; // Hash to store color counts

    // Check if the image size is below the cutoff
    if (std::min(image.height(), image.width()) <= CUTOFF) {
        return findMostCommonColors(image); // Process the image using the serial function
    }
    else {
        // Divide the image into 4 parts
        QRect topLeftRect(0, 0, image.width() / 2, image.height() / 2);
        QRect topRightRect(image.width() / 2, 0, image.width() / 2, image.height() / 2);
        QRect bottomLeftRect(0, image.height() / 2, image.width() / 2, image.height() / 2);
        QRect bottomRightRect(image.width() / 2, image.height() / 2, image.width() / 2, image.height() / 2);

        // Create a task group
        tbb::task_group taskGroup;

        // Run tasks for each image part
        taskGroup.run([&] { partialResults[0] = findMostCommonColorsParallel(image.copy(topLeftRect)); });
        taskGroup.run([&] { partialResults[1] = findMostCommonColorsParallel(image.copy(topRightRect)); });
        taskGroup.run([&] { partialResults[2] = findMostCommonColorsParallel(image.copy(bottomLeftRect)); });
        taskGroup.run([&] { partialResults[3] = findMostCommonColorsParallel(image.copy(bottomRightRect)); });

        // Wait for all tasks to complete
        taskGroup.wait();

        // Populate colorCounts hash with color counts from partial results
        for (const auto& partialResult : partialResults) {
            for (const auto& color : partialResult) {
                colorCounts[color]++;
            }
        }

        // Merge the partial results
        for (const auto& partialResult : partialResults) {
            mostCommonColors += partialResult;
        }

        // Sort the colors based on their occurrence count
        QList<QColor> sortedColors = colorCounts.keys();
        std::sort(sortedColors.begin(), sortedColors.end(), [&](const QColor& color1, const QColor& color2) {
            return colorCounts[color1] > colorCounts[color2];
            });

        // Keep only the top 3 most common colors
        if (mostCommonColors.size() > 3) {
            mostCommonColors.resize(3);
        }

        return mostCommonColors;
    }
}


//function for getting similar colors for changing
bool ImageColorChange::isSimilarColor(const QColor& color1, const QColor& color2, int tolerance) {

    // Calculate the color difference for each channel (RGB)
    int redDiff = qAbs(color1.red() - color2.red());
    int greenDiff = qAbs(color1.green() - color2.green());
    int blueDiff = qAbs(color1.blue() - color2.blue());

    // Check if the color difference is within the specified tolerance range
    return (redDiff <= tolerance && greenDiff <= tolerance && blueDiff <= tolerance);
}

//serial function for replace color
void ImageColorChange::replaceColor(QImage& image, const QColor& targetColor, const QColor& replacementColor, int colorTolerance)
{
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor pixelColor(image.pixel(x, y));

            // Check if the pixel color is within the tolerance range of the target color
            if (isSimilarColor(pixelColor, targetColor, colorTolerance)) {
                // Replace the pixel color with the replacement color
                image.setPixel(x, y, replacementColor.rgb());
            }
        }
    }
}

//parallel function for replace color
void ImageColorChange::replaceColorParallel(QImage& image, const QColor& targetColor, const QColor& replacementColor, int colorTolerance)
{
    // Check if the image size is below the cutoff
    if (std::min(image.height(), image.width()) <= CUTOFF) {
        return replaceColor(image, targetColor, replacementColor, colorTolerance); // Process the image using the serial function
    }
    else {
        // Divide the image into 4 parts
        QRect topLeftRect(0, 0, image.width() / 2, image.height() / 2);
        QRect topRightRect(image.width() / 2, 0, image.width() / 2, image.height() / 2);
        QRect bottomLeftRect(0, image.height() / 2, image.width() / 2, image.height() / 2);
        QRect bottomRightRect(image.width() / 2, image.height() / 2, image.width() / 2, image.height() / 2);

        // Create a task group
        tbb::task_group taskGroup;

        // Run tasks for each image part
        QImage topLeftImage = image.copy(topLeftRect);
        QImage topRightImage = image.copy(topRightRect);
        QImage bottomLeftImage = image.copy(bottomLeftRect);
        QImage bottomRightImage = image.copy(bottomRightRect);

        taskGroup.run([&] { replaceColorParallel(topLeftImage, targetColor, replacementColor, colorTolerance); });
        taskGroup.run([&] { replaceColorParallel(topRightImage, targetColor, replacementColor, colorTolerance); });
        taskGroup.run([&] { replaceColorParallel(bottomLeftImage, targetColor, replacementColor, colorTolerance); });
        taskGroup.run([&] { replaceColorParallel(bottomRightImage, targetColor, replacementColor, colorTolerance); });


        // Wait for all tasks to complete
        taskGroup.wait();

    }
}


//serial function for filter apply
void ImageColorChange::applyFilter(QImage& sourceImage) {

    int kernelSize = 7;
    double sigma = 5;

    int radius = kernelSize / 2;

    // Calculate the kernel values
    std::vector<double> kernel(kernelSize * kernelSize);
    double sum = 0.0;
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            double value = std::exp(-(x * x + y * y) / (2 * sigma * sigma));
            kernel[(y + radius) * kernelSize + (x + radius)] = value;
            sum += value;
        }
    }

    // Normalize the kernel values
    for (double& value : kernel) {
        value /= sum;
    }

    // Apply the Gaussian blur filter to the image
    for (int y = 0; y < sourceImage.height(); ++y) {
    for (int x = 0; x < sourceImage.width(); ++x) {
        double red = 0.0, green = 0.0, blue = 0.0, alpha = 0.0;

        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                int sx = qBound(0, x + dx, sourceImage.width() - 1);
                int sy = qBound(0, y + dy, sourceImage.height() - 1);

                QColor pixelColor(sourceImage.pixel(sx, sy));
                double weight = kernel[(dy + radius) * kernelSize + (dx + radius)];

                red += pixelColor.redF() * weight;
                green += pixelColor.greenF() * weight;
                blue += pixelColor.blueF() * weight;
                alpha += pixelColor.alphaF() * weight;
            }
        }

        QColor blurredColor(qRound(red * 255.0), qRound(green * 255.0), qRound(blue * 255.0), qRound(alpha * 255.0));
        sourceImage.setPixel(x, y, blurredColor.rgba());
    }
}
}

//parallel function for filter apply
void ImageColorChange::applyFilterParallel(QImage& image) {

    // Check if the image size is below the cutoff
    if (std::min(image.height(), image.width()) <= CUTOFF) {
        return applyFilter(image); // Process the image using the serial function
    }
    else {
        // Divide the image into 4 parts
        QRect topLeftRect(0, 0, image.width() / 2, image.height() / 2);
        QRect topRightRect(image.width() / 2, 0, image.width() / 2, image.height() / 2);
        QRect bottomLeftRect(0, image.height() / 2, image.width() / 2, image.height() / 2);
        QRect bottomRightRect(image.width() / 2, image.height() / 2, image.width() / 2, image.height() / 2);

        // Create a task group
        tbb::task_group taskGroup;

        // Run tasks for each image part
        QImage topLeftImage = image.copy(topLeftRect);
        QImage topRightImage = image.copy(topRightRect);
        QImage bottomLeftImage = image.copy(bottomLeftRect);
        QImage bottomRightImage = image.copy(bottomRightRect);

        taskGroup.run([&] { applyFilterParallel(topLeftImage); });
        taskGroup.run([&] { applyFilterParallel(topRightImage); });
        taskGroup.run([&] { applyFilterParallel(bottomLeftImage); });
        taskGroup.run([&] { applyFilterParallel(bottomRightImage); });


        // Wait for all tasks to complete
        taskGroup.wait();

    }


}