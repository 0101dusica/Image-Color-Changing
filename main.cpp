#include "ImageColorChange.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ImageColorChange window;
    window.show();

    return app.exec();
}