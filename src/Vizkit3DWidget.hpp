#ifndef __VIZKIT_QVIZKITWIDGET__
#define __VIZKIT_QVIZKITWIDGET__

#include <vizkit/CompositeViewerQOSG.hpp>
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
    class QProperyBrowserWidget;
    class Vizkit3DView;

class QDESIGNER_WIDGET_EXPORT Vizkit3DWidget : public CompositeViewerQOSG 
{
    Q_OBJECT

public:
    Vizkit3DWidget( QWidget* parent = 0, Qt::WindowFlags f = 0 );
    
    /** Defined to avoid unnecessary dependencies in the headers
     *
     * If it is not defined explicitely, GCC will try to emit it inline, which
     * means that the types used in the osg_ptr below must be defined.
     */
    ~Vizkit3DWidget();
    
    /**
     * Sets the camera focus to specific position.
     * @param lookAtPos focus this point
     */
    void changeCameraView(const osg::Vec3& lookAtPos);

    /**
     * Sets the camera focus and the camera itself to specific position.
     * @param lookAtPos focus this point
     * @param eyePos position of the camera
     */
    void changeCameraView(const osg::Vec3& lookAtPos, const osg::Vec3& eyePos);

public slots:
    /** Add new vizkit plugins to this view
     *
     * The plugins are added recursively, and the given plugin does not
     * necessarily have to be a vizkit plugin. It only has to have children that
     * are vizkit plugins
     */
    void addPlugin(QObject* plugin, QObject* parent = NULL);

    /** Remove a vizkit plugin from this view
     */
    void removePlugin(QObject* plugin);
    
    /** Make the camera track the root of this plugin
     */
    void setTrackedNode(VizPluginBase* plugin );

    /**
     * Sets the frame in which the camera is fixed
     */
    void setVisualizationFrame(const QString &frame);
    
    /**
     * Sets frame plugin data for a given plugin.
     * The pluging data frame is the frame in which the 
     * plugin expects the data to be.  
     * e.g. in case of the LaserScanVisualization 'laser'
     */
    void setPluginDataFrame(const QString &frame, VizPluginBase *plugin);
    
    /**
     * Adds a dynamic transformation to this view's transformer
     */
    void pushDynamicTransformation(const base::samples::RigidBodyState &tr);
	
    /**
     * Adds a static transformation to this view's transformer
     */
    void pushStaticTransformation(const base::samples::RigidBodyState &tr);

    bool isGridEnabled() const;
    void setGridEnabled(bool enabled);
    bool areAxesEnabled() const;
    void setAxesEnabled(bool enabled);

    QWidget* getPropertyWidget();

    void setCameraLookAt(double x, double y, double z);
    void setCameraEye(double x, double y, double z);
    void setCameraUp(double x, double y, double z);

protected slots:
    void updatedTransformation(std::string const& source, std::string const& target);
    
protected:
    void autoAddFrameToCombo(const std::string& frame);
    QWidget* createControlPane();
    void registerOnPropertyBrowser(QObject* plugin, QObject* parent);
    void deregisterFromPropertyBrowser(QObject* plugin);

    QProperyBrowserWidget* propertyBrowserWidget;
    QComboBox *frameSelector;
    QGroupBox* groupBox;
    Vizkit3DView* vizkitView;
    osg::ref_ptr<osgViewer::View> osgView;
    /** The set of frames currently known to exist */
    std::set<std::string> availableFrames;
};

}

#endif
