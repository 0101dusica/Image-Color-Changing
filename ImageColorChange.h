#ifndef IMAGECOLORCHANGE_H
#define IMAGECOLORCHANGE_H

#define QT_NO_KEYWORDS  // Define QT_NO_KEYWORDS before including any Qt headers

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>

class ImageColorChange : public QMainWindow
{
    Q_OBJECT

public:
    ImageColorChange(QWidget* parent = nullptr);

private:
    QLabel* createColorLabel();
    QLabel* createTimeLabel();

    QLabel* imageLabel;
    QPushButton* importButton;
    QLabel* colorLabel1;
    QLabel* colorLabel2;
    QLabel* colorLabel3;
    QPushButton* pickerButton1;
    QPushButton* pickerButton2;
    QPushButton* changeColorButton;
    QPushButton* filterButton;
    QLabel* timeLabel1;
    QLabel* timeLabel2;
    QLabel* timeLabel3;
    QLabel* timeLabelParallel1;
    QLabel* timeLabelParallel2;
    QLabel* timeLabelParallel3;


    bool isSimilarColor(const QColor& color1, const QColor& color2, int tolerance);
    void replaceColor(QImage& image, const QColor& targetColor, const QColor& replacementColor, int colorTolerance);
    void replaceColorParallel(QImage& image, const QColor& targetColor, const QColor& replacementColor, int colorTolerance);

    void applyFilter(QImage& sourceImage);
    void applyFilterParallel(QImage& sourceImage);

    QVector<QColor> findMostCommonColors(const QImage& image);
    QVector<QColor> findMostCommonColorsParallel(const QImage& image);


private:
    void openImageDialog();
    void pickColor1();
    void pickColor2();

    void mostCommonColorsGUI();
    void colorChangingGUI();
    void applyFilterGUI();
};

#endif // IMAGECOLORCHANGE_H

