#ifndef VISP_TRACKER_TRACKER_CLIENT_HH
# define VISP_TRACKER_TRACKER_CLIENT_HH
# include <boost/filesystem/fstream.hpp>
# include <boost/filesystem/path.hpp>
# include <boost/thread/recursive_mutex.hpp>

# include <dynamic_reconfigure/server.h>

# include <image_proc/advertisement_checker.h>

# include <image_transport/image_transport.h>
# include <image_transport/subscriber_filter.h>

# include <message_filters/subscriber.h>
# include <message_filters/sync_policies/approximate_time.h>
# include <message_filters/synchronizer.h>

# include <sensor_msgs/Image.h>
# include <sensor_msgs/CameraInfo.h>

# include <resource_retriever/retriever.h>

# include <visp_tracker/ModelBasedSettingsConfig.h>
# include <visp_tracker/MovingEdgeSites.h>

# include <visp/vpCameraParameters.h>
# include <visp/vpHomogeneousMatrix.h>
# include <visp/vpImage.h>
# include <visp/vpMbTracker.h>
# include <visp/vpMe.h>
# include <visp/vpKltOpencv.h>
# include <visp/vpPose.h>

#include "detectors/detector_base.h"

namespace visp_tracker
{
  class TrackerClient
  {
  public:
    typedef vpImage<unsigned char> image_t;
    typedef std::vector<vpPoint> points_t;
    typedef std::vector<vpImagePoint> imagePoints_t;

    typedef dynamic_reconfigure::Server<visp_tracker::ModelBasedSettingsConfig>
    reconfigureSrv_t;

    typedef dynamic_reconfigure::Server
    <visp_tracker::ModelBasedSettingsConfig>::CallbackType
    reconfigureCallback_t;


    TrackerClient(ros::NodeHandle& nh,
		  ros::NodeHandle& privateNh,
		  volatile bool& exiting,
		  unsigned queueSize = 5u);
    
    ~TrackerClient();

    void spin();
  protected:
    /// \brief Make sure the topics we subscribe already exist.
    void checkInputs();

    void loadModel();

    bool validatePose(const vpHomogeneousMatrix& cMo);
    vpHomogeneousMatrix loadInitialPose();
    vpHomogeneousMatrix loadPose(const std::string& fileName);
    void saveInitialPose(const vpHomogeneousMatrix& cMo);
    points_t loadInitializationPoints();

    void init();
    void initPoint(unsigned& i,
		   points_t& points,
		   imagePoints_t& imagePoints,
		   ros::Rate& rate,
		   vpPose& pose);


    void waitForImage();

    void sendcMo(const vpHomogeneousMatrix& cMo);

    std::string fetchResource(const std::string&);
    bool makeModelFile(boost::filesystem::ofstream& modelStream,
		       const std::string& resourcePath,
		       std::string& fullModelPath);

  private:
    bool exiting ()
    {
      return exiting_ || !ros::ok();
    }

    void find_flashcode_pos(vpHomogeneousMatrix& cMo);
    void initFromFlashCode();

    volatile bool& exiting_;

    unsigned queueSize_;

    ros::NodeHandle& nodeHandle_;
    ros::NodeHandle& nodeHandlePrivate_;

    image_transport::ImageTransport imageTransport_;

    image_t image_;

    std::string modelPath_;
    std::string modelName_;

    std::string cameraPrefix_;
    std::string rectifiedImageTopic_;
    std::string cameraInfoTopic_;
    std::string trackerType_;

    boost::filesystem::path vrmlPath_;
    boost::filesystem::path initPath_;

    image_transport::CameraSubscriber cameraSubscriber_;

    boost::recursive_mutex mutex_;
    reconfigureSrv_t reconfigureSrv_;

    std_msgs::Header header_;
    sensor_msgs::CameraInfoConstPtr info_;

    vpMe movingEdge_;
    vpKltOpencv kltTracker_;
    vpCameraParameters cameraParameters_;
    vpMbTracker *tracker_;

    bool startFromSavedPose_;
    bool confirmInit_;
    bool useFlashCodeQRCode_;
    bool useFlashCodeDataMatrix_;
    double flashcode_size_;
    detectors::DetectorBase* detector_;

    /// \brief Helper used to check that subscribed topics exist.
    image_proc::AdvertisementChecker checkInputs_;

    resource_retriever::Retriever resourceRetriever_;
  };
} // end of namespace visp_tracker.

#endif //! VISP_TRACKER_TRACKER_CLIENT_HH
