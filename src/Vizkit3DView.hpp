#ifndef VIZKIT_VIZKIT3DVIEW_HPP
#define VIZKIT_VIZKIT3DVIEW_HPP

#include <vizkit/QOSGWidget.hpp>
#include <QtDesigner/QDesignerExportWidget>
#include <transformer/NonAligningTransformer.hpp>
#include <vizkit/Vizkit3DPlugin.hpp>

class QComboBox;
class QGroupBox;

namespace vizkit 
{
    class PickHandler;
    class CoordinateFrame;
    class GridNode;

/** 3D view widget that allows to manage the vizkit3d plugins that are attached
 * on it. 
 *
 * The main difference with Vizkit3DWidget is that it does not have additional
 * controls (e.g. property browser, ...)
 */
class QDESIGNER_WIDGET_EXPORT Vizkit3DView : public ViewQOSG
{
    Q_OBJECT
    Q_PROPERTY(bool show_grid READ isGridEnabled WRITE setGridEnabled)
    Q_PROPERTY(bool show_axes READ areAxesEnabled WRITE setAxesEnabled)

    /**
     * Book-keeping class for the plugin transformations
     * */
    struct TransformationData
    {
        TransformationData() : transformation(NULL) {};

        QString dataFrame;
        transformer::Transformation *transformation;
    };
    
public:
    Vizkit3DView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
    
    /** Defined to avoid unnecessary dependencies in the headers
     *
     * If it is not defined explicitely, GCC will try to emit it inline, which
     * means that the types used in the osg_ptr below must be defined.
     */
    ~Vizkit3DView();

    /** Return the root node of this view
     */
    osg::ref_ptr<osg::Group> getRootNode() const;

    QSize sizeHint() const;
    
public slots:
    /** Add a vizkit plugin to this view */
    void addPlugin(QObject* plugin, QObject* parent = NULL);

    /** Remove a vizkit plugin from this view */
    void removePlugin(QObject* plugin);
    
    /** Make the camera track the root node of this plugin */
    void setTrackedNode( VizPluginBase* plugin );

    /** Returns the current visualization frame */
    QString getVisualizationFrame() const;
    /** Sets the frame in which the view's camera is fixed */
    void setVisualizationFrame(const QString &frame);
    
    /**
     * Sets the frame of a given plugin's root node
     *
     * The pluging data frame is the frame in which the plugin expects the data
     * to be. For instance, in case of the LaserScanVisualization plugin, the
     * frame of the laser scanner that generates the laser scan.
     */
    void setPluginDataFrame(const QString &frame, QObject *plugin);
    
    /** Update a dynamic transformation
     */
    void pushDynamicTransformation(const base::samples::RigidBodyState &tr);
	
    /** Gives information about a static transformation
     */
    void pushStaticTransformation(const base::samples::RigidBodyState &tr);

    /** If true, the default grid visualization plugin is currently displayed */
    bool isGridEnabled() const;
    /** Changes whether the default grid visualization is enabled or not */
    void setGridEnabled(bool enabled);
    /** If true, the default axes visualization plugin is currently displayed */
    bool areAxesEnabled() const;
    /** Changes whether the default axes visualization is enabled or not */
    void setAxesEnabled(bool enabled);
    
signals:
    /** Helper signal, do not use */
    void addPlugins(QObject* plugin, QObject* parent);
    /** Helper signal, do not use */
    void removePlugins(QObject* plugin);
    /** Emitted when one of this object's property changed */
    void propertyChanged(QString propertyName);
    /** Notifies that the given transformation has been pushed either in
     * pushDynamicTransformation or pushStaticTransformation
     */
    void updatedTransformation(QString const& sourceFrame, QString const& targetFrame);
    
private slots:
    void addPluginIntern(QObject* plugin,QObject *parent=NULL);
    void removePluginIntern(QObject* plugin);
    void pluginActivityChanged(bool enabled);

protected:
    void checkAddFrame(const QString &frame);

    /** Adds the root node of this visualization plugin to the OSG graph */
    void addDataHandler(VizPluginBase *viz);
    /** Removes the root node of this visualization plugin from the OSG graph */
    void removeDataHandler(VizPluginBase *viz);
    /** Sets the transformation of each plugin's root node based on the current
     * transformer state
     */
    void updateTransformations();

    /** Updates the transformation object in \c td based on the given target
     * frame and the current display frame
     */
    void updatePluginTransformation(TransformationData& td, const QString& frame);
    
    osg::ref_ptr<osg::Group> root;
    void createSceneGraph();
    osg::ref_ptr<PickHandler> pickHandler;
    osg::ref_ptr<GridNode> groundGrid;
    osg::ref_ptr<CoordinateFrame> coordinateFrame;
    transformer::NonAligningTransformer transformer;
    
    QString visualizationFrame;
    std::vector<VizPluginBase *> plugins;
    
    typedef std::map<vizkit::VizPluginBase*, TransformationData> PluginToTransform;
    PluginToTransform pluginToTransformData;
};

}

#endif
