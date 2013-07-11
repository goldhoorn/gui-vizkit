#include "Vizkit3DView.hpp"
#include <QVBoxLayout>
#include <QSplitter>
#include <QComboBox>
#include <QGroupBox>

#include <vizkit/QOSGWidget.hpp>
#include <vizkit/Vizkit3DPlugin.hpp>
#include <vizkit/GridNode.hpp>
#include <vizkit/CoordinateFrame.hpp>
#include <vizkit/PickHandler.hpp>

using namespace vizkit;

Vizkit3DView::Vizkit3DView( QWidget* parent, Qt::WindowFlags f )
    : ViewQOSG( parent )
{
    createSceneGraph();

    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    setData(getRootNode());

    // pickhandler is for selecting objects in the opengl view
    pickHandler = new PickHandler();
    addEventHandler( pickHandler );
    
    // set root node as default tracked node
    ViewQOSG::setTrackedNode(getRootNode());
    
    // create visualization for default features
    groundGrid = new GridNode();
    coordinateFrame = new CoordinateFrame();
    // And enable only the grid by default
    setGridEnabled(true);
    
    changeCameraView(osg::Vec3d(0,0,0), osg::Vec3d(-5,0,5));
    
    connect(this, SIGNAL(addPlugins(QObject*,QObject*)), this, SLOT(addPluginIntern(QObject*,QObject*)));
    connect(this, SIGNAL(removePlugins(QObject*)), this, SLOT(removePluginIntern(QObject*)));
}

Vizkit3DView::~Vizkit3DView() {}

QSize Vizkit3DView::sizeHint() const
{
    return QSize( 1000, 600 );
}

osg::ref_ptr<osg::Group> Vizkit3DView::getRootNode() const
{
    return root;
}

void Vizkit3DView::setTrackedNode( VizPluginBase* plugin )
{
    ViewQOSG::setTrackedNode(plugin->getRootNode());
}

void Vizkit3DView::createSceneGraph() 
{
    //create root node that holds all other nodes
    root = new osg::Group;
    
    osg::ref_ptr<osg::StateSet> state = root->getOrCreateStateSet();
    state->setGlobalDefaults();
    state->setMode( GL_LINE_SMOOTH, osg::StateAttribute::ON );
    state->setMode( GL_POINT_SMOOTH, osg::StateAttribute::ON );
    state->setMode( GL_BLEND, osg::StateAttribute::ON );    
    state->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON);
    state->setMode( GL_LIGHTING, osg::StateAttribute::ON );
    state->setMode( GL_LIGHT0, osg::StateAttribute::ON );
    state->setMode( GL_LIGHT1, osg::StateAttribute::ON );
	
    root->setDataVariance(osg::Object::DYNAMIC);

    // Add the Light to a LightSource. Add the LightSource and
    //   MatrixTransform to the scene graph.
    for(size_t i=0;i<2;i++)
    {
	osg::ref_ptr<osg::Light> light = new osg::Light;
	light->setLightNum(i);
	switch(i) {
	    case 0:
		light->setAmbient( osg::Vec4( .1f, .1f, .1f, 1.f ));
		light->setDiffuse( osg::Vec4( .8f, .8f, .8f, 1.f ));
		light->setSpecular( osg::Vec4( .8f, .8f, .8f, 1.f ));
		light->setPosition( osg::Vec4( 1.f, 1.5f, 2.f, 0.f ));
		break;
	    case 1:
		light->setAmbient( osg::Vec4( .1f, .1f, .1f, 1.f ));
		light->setDiffuse( osg::Vec4( .1f, .3f, .1f, 1.f ));
		light->setSpecular( osg::Vec4( .1f, .3f, .1f, 1.f ));
		light->setPosition( osg::Vec4( -1.f, -3.f, 1.f, 0.f ));
	}

	osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
	ls->setLight( light.get() );
	//ls->setStateSetModes(*state, osg::StateAttribute::ON);
	root->addChild( ls.get() );
    }
}

void Vizkit3DView::addDataHandler(VizPluginBase *viz)
{
    root->addChild( viz->getRootNode() );
}

void Vizkit3DView::removeDataHandler(VizPluginBase *viz)
{
    root->removeChild( viz->getRootNode() );
}

/**
 * Puts the plugin in a list and emits a signal.
 * Adding the new Plugin will be handled by the main thread.
 * @param plugin Vizkit Plugin
 */
void Vizkit3DView::addPlugin(QObject* plugin,QObject *parent)
{
    emit addPlugins(plugin, parent);
}

void Vizkit3DView::removePlugin(QObject* plugin)
{
    emit removePlugins(plugin);
}

void Vizkit3DView::addPluginIntern(QObject* plugin, QObject *parent)
{
    VizPluginBase* viz_plugin = dynamic_cast<VizPluginBase*>(plugin);
    if (viz_plugin)
    {
        if (!parent)
            viz_plugin->setParent(this);
        plugins.push_back(viz_plugin);
	
	if(pluginToTransformData.count(viz_plugin))
	    throw std::runtime_error("Error added same plugin twice");
	
	pluginToTransformData[viz_plugin] = TransformationData();
        addDataHandler(viz_plugin);
        connect(plugin, SIGNAL(pluginActivityChanged(bool)),
                this, SLOT(pluginActivityChanged(bool)));
    }

    QList<QObject*> object_list = plugin->children();
    for(QList<QObject*>::const_iterator it = object_list.begin();
            it != object_list.end(); it++)
        addPluginIntern(*it, plugin);
}

void Vizkit3DView::removePluginIntern(QObject* plugin)
{
    VizPluginBase* viz_plugin = dynamic_cast<VizPluginBase*>(plugin);
    if (viz_plugin)
    {
        std::vector<vizkit::VizPluginBase *>::iterator it =
            std::find(plugins.begin(), plugins.end(), viz_plugin);
        if(it == plugins.end())
            throw std::runtime_error("Tried to remove an vizkit3d plugin that was not registered before");

        removeDataHandler(viz_plugin);
        disconnect(viz_plugin, SIGNAL(pluginActivityChanged(bool)),
                this, SLOT(pluginActivityChanged(bool)));

	pluginToTransformData.erase(viz_plugin);
        plugins.erase(it);
    }

    QList<QObject*> object_list = plugin->children();
    for(QList<QObject*>::const_iterator it = object_list.begin();
            it != object_list.end(); it++)
        removePluginIntern(*it);
}


void Vizkit3DView::setPluginDataFrame(const QString& frame, QObject* plugin)
{
    VizPluginBase* viz_plugin = dynamic_cast<VizPluginBase*>(plugin);
    if(!viz_plugin)
	throw std::runtime_error("setPluginDataFrame called with something that is no vizkit plugin");
    if(pluginToTransformData.count(viz_plugin) == 0)
        throw std::runtime_error("Tried to set frame for unknown plugin");
    
    TransformationData& td = pluginToTransformData[viz_plugin]; 
    if(td.dataFrame == frame)
	return;

    updatePluginTransformation(td, frame);
}

void Vizkit3DView::updatePluginTransformation(TransformationData& td, const QString& frame)
{
    if(td.transformation)
        transformer.unregisterTransformation(td.transformation);
    td.dataFrame = frame;
    if(!visualizationFrame.isEmpty())
        td.transformation = &transformer.registerTransformation(
                frame.toStdString(), visualizationFrame.toStdString());
    else
        td.transformation = NULL;
}

QString Vizkit3DView::getVisualizationFrame() const
{
    return visualizationFrame;
}

void Vizkit3DView::setVisualizationFrame(const QString& frame)
{
    visualizationFrame = frame;
    for(PluginToTransform::iterator it = pluginToTransformData.begin();
            it != pluginToTransformData.end(); it++)
    {
        TransformationData &data(it->second);
        updatePluginTransformation(data, data.dataFrame);
    }
    updateTransformations();
}

void Vizkit3DView::pushDynamicTransformation(const base::samples::RigidBodyState& tr)
{
    if(!tr.hasValidPosition() || !tr.hasValidOrientation())
    {
        std::cerr << "Vizkit3DView ignoring invalid dynamic transformation " << tr.sourceFrame << " --> " << tr.targetFrame << std::endl;
        return;
    }

    transformer.pushDynamicTransformation(tr);
    while(transformer.step())
    {
	;
    }
    updateTransformations();
    emit updatedTransformation(
            QString::fromStdString(tr.sourceFrame),
            QString::fromStdString(tr.targetFrame));
}

void Vizkit3DView::updateTransformations()
{
    for(std::map<vizkit::VizPluginBase *, TransformationData>::iterator it = pluginToTransformData.begin(); it != pluginToTransformData.end(); it++)
    {
        TransformationData &data(it->second);
	if(data.transformation)
	{
	    Eigen::Affine3d pose;
	    if(data.transformation->get(base::Time(), pose, false))
	    {
		it->first->setPose(pose.translation(), Eigen::Quaterniond(pose.rotation()));
	    }
	}
    }
}

void Vizkit3DView::pushStaticTransformation(const base::samples::RigidBodyState& tr)
{
    if(!tr.hasValidPosition() || !tr.hasValidOrientation())
    {
        std::cerr << "Vizkit3DView ignoring invalid static transformation " << tr.sourceFrame << " --> " << tr.targetFrame << std::endl;
        return;
    }
    transformer.pushStaticTransformation(tr);
    emit updatedTransformation(
            QString::fromStdString(tr.sourceFrame),
            QString::fromStdString(tr.targetFrame));
}

void Vizkit3DView::pluginActivityChanged(bool enabled)
{
    QObject* obj = QObject::sender();
    vizkit::VizPluginBase* viz_plugin = dynamic_cast<vizkit::VizPluginBase*>(obj);
    if(viz_plugin)
    {
        // check if root node has plugin as child
        bool has_child_plugin = false;
        if(root->getChildIndex(viz_plugin->getRootNode()) < root->getNumChildren())
            has_child_plugin = true;
        
        // add or remove plugin from root node
        if(enabled && !has_child_plugin)
        {
            addDataHandler(viz_plugin);
        }
        else if (!enabled && has_child_plugin)
        {
            removeDataHandler(viz_plugin);
        }
    }
}

/**
 * @return true if ground grid is enabled
 */
bool Vizkit3DView::isGridEnabled() const
{
    return (root->getChildIndex(groundGrid) < root->getNumChildren());
}

/**
 * Enable or disable ground grid.
 * @param enabled
 */
void Vizkit3DView::setGridEnabled(bool enabled)
{
    if(!enabled && isGridEnabled())
    {
        root->removeChild(groundGrid);
    }
    else if(enabled && !isGridEnabled())
    {
        root->addChild(groundGrid);
    }
    emit propertyChanged("show_grid");
}

/**
 * @return true if axes coordinates are enabled
 */
bool Vizkit3DView::areAxesEnabled() const
{
    return (root->getChildIndex(coordinateFrame) < root->getNumChildren());
}

/**
 * Enable or disable axes of the coordinate system.
 * @param enabled
 */
void Vizkit3DView::setAxesEnabled(bool enabled)
{
    if(!enabled && areAxesEnabled())
    {
        root->removeChild(coordinateFrame);
    }
    else if(enabled && !areAxesEnabled())
    {
        root->addChild(coordinateFrame);
    }
    emit propertyChanged("show_axes");
}
