#include "Vizkit3DWidget.hpp"
#include <QVBoxLayout>
#include <QSplitter>
#include <QComboBox>
#include <QGroupBox>
#include <QPlastiqueStyle>

#include <vizkit/QOSGWidget.hpp>
#include <vizkit/Vizkit3DPlugin.hpp>
#include <vizkit/GridNode.hpp>
#include <vizkit/CoordinateFrame.hpp>
#include <vizkit/PickHandler.hpp>
#include <vizkit/QPropertyBrowserWidget.hpp>
#include <vizkit/Vizkit3DView.hpp>

using namespace vizkit;
using namespace std;

Vizkit3DWidget::Vizkit3DWidget( QWidget* parent, Qt::WindowFlags f )
    : CompositeViewerQOSG( parent, f )
{
    QHBoxLayout* mainLayout = new QHBoxLayout;
    setLayout(mainLayout);
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(splitter);

    QWidget* controlWidget = createControlPane();
    splitter->addWidget(controlWidget);
    vizkitView = new Vizkit3DView(this);
    osgView = vizkitView; // needed to keep reference counting on the OSG side
    addView( osgView );
    splitter->addWidget(vizkitView);
    
    // add some properties of this widget as global properties
    QStringList property_names("show_grid");
    property_names.push_back("show_axes");
    propertyBrowserWidget->addGlobalProperties(vizkitView, property_names);
    
    connect(frameSelector, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(setVisualizationFrame(QString)));
}

Vizkit3DWidget::~Vizkit3DWidget() {}

QWidget* Vizkit3DWidget::createControlPane()
{
    QWidget* controlWidget = new QWidget(this);
    QVBoxLayout* controlLayout = new QVBoxLayout;
    controlWidget->setLayout(controlLayout);

    // Create and add the frame selection combo
    frameSelector = new QComboBox();
    groupBox      = new QGroupBox();
    QVBoxLayout* groupBoxLayout = new QVBoxLayout;
    groupBox->setLayout(groupBoxLayout);
    groupBox->setTitle("Select Visualization Frame");
    groupBox->setEnabled(false);
    groupBoxLayout->addWidget(frameSelector);
    controlLayout->addWidget(groupBox);

    // Create and add the property browser
    propertyBrowserWidget = new QProperyBrowserWidget();
    propertyBrowserWidget->setSizePolicy(
            QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    controlLayout->addWidget(propertyBrowserWidget);
    return controlWidget;
}

void Vizkit3DWidget::updatedTransformation(string const& source, string const& target)
{
    autoAddFrameToCombo(source);
    autoAddFrameToCombo(target);
}

void Vizkit3DWidget::autoAddFrameToCombo(const string& frame)
{
    pair<set<string>::iterator,bool> result = availableFrames.insert(frame);
    if(result.second)
    {
	frameSelector->addItem(QString::fromStdString(frame));
	if(frame == vizkitView->getVisualizationFrame().toStdString())
	{
	    int index = frameSelector->findText(QString::fromStdString(frame));
	    if(index != -1) {
		frameSelector->setCurrentIndex(index);
	    }
	}
	// enable frame selector groupBox if needed
	if(!groupBox->isEnabled())
        {
            groupBox->setEnabled(true);
        }
    }
}


void Vizkit3DWidget::setTrackedNode( VizPluginBase* plugin )
{
    vizkitView->setTrackedNode(plugin);
}

void Vizkit3DWidget::changeCameraView(const osg::Vec3& lookAtPos)
{
    vizkitView->changeCameraView(lookAtPos);
}

void Vizkit3DWidget::changeCameraView(const osg::Vec3& lookAtPos, const osg::Vec3& eyePos)
{
    vizkitView->changeCameraView(lookAtPos, eyePos);
}

void Vizkit3DWidget::setCameraLookAt(double x, double y, double z)
{
    vizkitView->setCameraLookAt(x, y, z);
}
void Vizkit3DWidget::setCameraEye(double x, double y, double z)
{
    vizkitView->setCameraEye(x, y, z);
}
void Vizkit3DWidget::setCameraUp(double x, double y, double z)
{
    vizkitView->setCameraUp(x, y, z);
}

void Vizkit3DWidget::addPlugin(QObject* plugin, QObject*parent)
{
    vizkitView->addPlugin(plugin, parent);
    registerOnPropertyBrowser(plugin, parent);
}

void Vizkit3DWidget::registerOnPropertyBrowser(QObject* plugin, QObject* parent)
{
    propertyBrowserWidget->addProperties(plugin, parent);

    // add sub plugins if object has some
    QList<QObject*> object_list = plugin->children();
    for(QList<QObject*>::const_iterator it = object_list.begin();
            it != object_list.end(); it++)
        registerOnPropertyBrowser(*it, plugin);
}

void Vizkit3DWidget::removePlugin(QObject* plugin)
{
    vizkitView->removePlugin(plugin);
    deregisterFromPropertyBrowser(plugin);
}

void Vizkit3DWidget::deregisterFromPropertyBrowser(QObject* plugin)
{
    propertyBrowserWidget->removeProperties(plugin);

    // add sub plugins if object has some
    QList<QObject*> object_list = plugin->children();
    for(QList<QObject*>::const_iterator it = object_list.begin();
            it != object_list.end(); it++)
        deregisterFromPropertyBrowser(*it);
}

void Vizkit3DWidget::setPluginDataFrame(const QString& frame, VizPluginBase* plugin)
{
    vizkitView->setPluginDataFrame(frame, plugin);
}

void Vizkit3DWidget::setVisualizationFrame(const QString& frame)
{
    vizkitView->setVisualizationFrame(frame);
}

void Vizkit3DWidget::pushDynamicTransformation(const base::samples::RigidBodyState& tr)
{
    vizkitView->pushDynamicTransformation(tr);
}

void Vizkit3DWidget::pushStaticTransformation(const base::samples::RigidBodyState& tr)
{
    vizkitView->pushStaticTransformation(tr);
}

QWidget* Vizkit3DWidget::getPropertyWidget()
{
    return propertyBrowserWidget;
}

bool Vizkit3DWidget::isGridEnabled() const
{
    return vizkitView->isGridEnabled();
}

void Vizkit3DWidget::setGridEnabled(bool enabled)
{
    return vizkitView->setGridEnabled(enabled);
}

bool Vizkit3DWidget::areAxesEnabled() const
{
    return vizkitView->areAxesEnabled();
}

void Vizkit3DWidget::setAxesEnabled(bool enabled)
{
    return vizkitView->setAxesEnabled(enabled);
}
