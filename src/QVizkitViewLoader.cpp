#include "QVizkitViewLoader.hpp"
#include <QtPlugin>
#include <vizkit/Vizkit3DView.hpp>

QVizkitViewLoader::QVizkitViewLoader(QObject* parent): QObject(parent)
{
    initialized = false;
}

void QVizkitViewLoader::initialize(QDesignerFormEditorInterface* core)
{
    if (initialized) return;

    initialized = true;
}

bool QVizkitViewLoader::isInitialized() const
{
    return initialized;
}

QWidget* QVizkitViewLoader::createWidget(QWidget* parent)
{
    return new vizkit::Vizkit3DView(parent, Qt::Widget);
}

QString QVizkitViewLoader::domXml() const
{
    return  "<ui language=\"c++\">\n"
            " <widget class=\"vizkit::Vizkit3DView\" name=\"Vizkit3DView\">\n"
            "  <property name=\"geometry\">\n"
            "   <rect>\n"
            "    <x>0</x>\n"
            "    <y>0</y>\n"
            "    <width>640</width>\n"
            "    <height>480</height>\n"
            "   </rect>\n"
            "  </property>\n"
            " </widget>\n"
            "</ui>\n";
}

bool QVizkitViewLoader::isContainer() const
{
    return false;
}

QIcon QVizkitViewLoader::icon() const
{
    return QIcon();
}

QString QVizkitViewLoader::includeFile() const
{
    return "vizkit/Vizkit3DView.hpp";
}

QString QVizkitViewLoader::name() const
{
    return "vizkit::Vizkit3DView";
}

QString QVizkitViewLoader::group() const
{
    return "Vizkit Widgets";
}

QString QVizkitViewLoader::toolTip() const
{
    return "";
}

QString QVizkitViewLoader::whatsThis() const
{
    return "";
}

Q_EXPORT_PLUGIN2(QVizkitViewLoader, QVizkitViewLoader)
