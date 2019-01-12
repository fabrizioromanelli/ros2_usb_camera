//
//  usb_camera_node.cpp
//  camera
//
//  Created by Andreas Klintberg on 11/17/18.
//  Copyright © 2018 Andreas Klintberg. All rights reserved.
//

#include <chrono>
#include <cstdio>
#include <memory>
#include <string>

#include "usb_camera_node.hpp"


using namespace std::chrono_literals;


/// Convert an OpenCV matrix encoding type to a string format recognized by sensor_msgs::Image.
/**
 * \param[in] mat_type The OpenCV encoding type.
 * \return A string representing the encoding type.
 */
std::string
mat_type2encoding(int mat_type)
{
    switch (mat_type) {
        case CV_8UC1:
            return "mono8";
        case CV_8UC3:
            return "bgr8";
        case CV_16SC1:
            return "mono16";
        case CV_8UC4:
            return "rgba8";
        default:
            throw std::runtime_error("Unsupported encoding type");
    }
}

Camera::~Camera(){}

Camera::Camera(const std::string & topic_name, const size_t & width, const size_t & height, double freq) :
Node("camera")
{
    i = 1;
    show_camera = false;
    is_flipped = false;
    
    // instanstiate message
    compressed_image_msg_ = std::make_shared<sensor_msgs::msg::CompressedImage>();
    
    // instanstiate publisher
    compressed_image_pub_ = this->create_publisher<sensor_msgs::msg::CompressedImage>(topic_name);
    std::string camera_info_url_ = "";
    std::string camera_name_ = "";
    
    //cinfo_manager_.reset(new camera_info_manager::CameraInfoManager(this, camera_name_, camera_info_url_));
    
    //camera_info_msg_ = std::make_shared<sensor_msgs::msg::CameraInfo>(cinfo_manager_.getCameraInfo());
    
    //cinfo->header.frame_id = config_.frame_id;
    //cinfo->header.stamp = timestamp;
    
    //camera_info_pub_ = image_pub_.advertiseCamera("image_raw", 1, false);
    
    cap.open(0);
    
    cap.set(CV_CAP_PROP_FRAME_WIDTH, static_cast<double>(width));
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, static_cast<double>(height));
    
    
    timer_ = this->create_wall_timer(100ms, std::bind(&Camera::ImageCallback, this));
    
    
    
}

void Camera::ImageCallback(){
    cap >> frame;
    if (!frame.empty()) {
        // Convert to a ROS image
        if (!is_flipped) {
            
            convert_frame_to_message(frame, i, compressed_image_msg_);
            convert_frame_to_message(frame, i, image_msg_);
            
        } else {
            // Flip the frame if needed
            cv::flip(frame, flipped_frame, 1);
            convert_frame_to_message(flipped_frame, i, compressed_image_msg_);
        }
        
        // Publish the image message and increment the frame_id.
        RCLCPP_INFO(this->get_logger(), "Publishing image #%zd", i);
        
        // Put the message into a queue to be processed by the middleware.
        // This call is non-blocking.
        compressed_image_pub_->publish(compressed_image_msg_);
        //stcamera_info_pub_.publish(image_msg_, camera_info_msg_);
        
        ++i;
    }
}


void Camera::convert_frame_to_message(
                                      const cv::Mat & frame, size_t frame_id, sensor_msgs::msg::CompressedImage::SharedPtr msg)
{
    
    std_msgs::msg::Header header;
    header.stamp = this->now();
    sensor_msgs::msg::CompressedImage img_msg;
    
    img_bridge = cv_bridge::CvImage(header, sensor_msgs::image_encodings::BGR8, frame);
    
    img_bridge.toCompressedImageMsg(img_msg); // from cv_bridge to sensor_msgs::CompressedImage
    msg->header =img_msg.header;
    msg->data = img_msg.data;
}


void Camera::convert_frame_to_message(
                                      const cv::Mat & frame, size_t frame_id, sensor_msgs::msg::Image::SharedPtr msg)
{
    
    std_msgs::msg::Header header;
    header.stamp = this->now();
    sensor_msgs::msg::Image img_msg;
    
    img_bridge = cv_bridge::CvImage(header, sensor_msgs::image_encodings::BGR8, frame);
    
    img_bridge.toImageMsg(img_msg); // from cv_bridge to sensor_msgs::Image
    msg->header =img_msg.header;
    msg->data = img_msg.data;
}


CameraNode::CameraNode(std::shared_ptr<rclcpp::Node>  const nh, std::shared_ptr<rclcpp::Node> const priv_nh) : nh_(nh), priv_nh_(priv_nh),
image_pub_(nh_),
cinfo_manager_(nh.get()) {
    camera_info_pub_ = image_pub_.advertiseCamera("image_raw", 1, false);
};


CameraNode::~CameraNode() {
    
}


void CameraNode::ConvertFrameToMessage(
                                      const cv::Mat & frame, size_t frame_id, sensor_msgs::msg::CompressedImage::SharedPtr msg)
{
    
    std_msgs::msg::Header header;
    //header.stamp = this->now();
    sensor_msgs::msg::CompressedImage img_msg;
    
    img_bridge = cv_bridge::CvImage(header, sensor_msgs::image_encodings::BGR8, frame);
    
    img_bridge.toCompressedImageMsg(img_msg); // from cv_bridge to sensor_msgs::CompressedImage
    msg->header =img_msg.header;
    msg->data = img_msg.data;
}

void CameraNode::ConvertFrameToMessage(
                                       const cv::Mat & frame, size_t frame_id, sensor_msgs::msg::Image::SharedPtr msg)
{
    
    std_msgs::msg::Header header;
    //header.stamp = this->now();
    sensor_msgs::msg::CompressedImage img_msg;
    
    img_bridge = cv_bridge::CvImage(header, sensor_msgs::image_encodings::BGR8, frame);
    
    img_bridge.toCompressedImageMsg(img_msg); // from cv_bridge to sensor_msgs::CompressedImage
    msg->header =img_msg.header;
    msg->data = img_msg.data;
}




void CameraNode::CaptureFrame() {
    cap >> frame;
    if (!frame.empty()) {
        // Convert to a ROS image
        if (!is_flipped) {
            
            //convert_frame_to_message(frame, i, compressed_image_msg_);
            ConvertFrameToMessage(frame, i, image_msg_);
            
        } else {
            // Flip the frame if needed
            cv::flip(frame, flipped_frame, 1);
            //convert_frame_to_message(flipped_frame, i, compressed_image_msg_);
            ConvertFrameToMessage(frame, i, image_msg_);
        }
        
        // Publish the image message and increment the frame_id.
        //RCLCPP_INFO(this->get_logger(), "Publishing image #%zd", i);
        
        // Put the message into a queue to be processed by the middleware.
        // This call is non-blocking.
        //compressed_image_pub_->publish(compressed_image_msg_);
        
        sensor_msgs::msg::CameraInfo::SharedPtr camera_info_msg_(
                                                      new sensor_msgs::msg::CameraInfo(cinfo_manager_.getCameraInfo()));
        camera_info_pub_.publish(image_msg_, camera_info_msg_);
        ++i;
    }
}

bool CameraNode::Start(){
    cap.open(0);
    
    size_t width = 320;
    size_t height = 240;
    
    cap.set(CV_CAP_PROP_FRAME_WIDTH, static_cast<double>(width));
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, static_cast<double>(height));
    
    //https://github.com/ros2/demos/blob/master/composition/src/talker_component.cpp
    
    //timer_ = create_wall_timer(100ms, std::bind(&ImageCallback, this));
    timer_ = nh_->create_wall_timer(100ms, std::bind(&CameraNode::CaptureFrame, this));

    return true;
}

int main(int argc, char * argv[])
{
    // based on libuvc
    std::cout << "Starting camera node" << std::endl;
    
    rclcpp::init(argc, argv);
    
    rclcpp::executors::SingleThreadedExecutor exec;

    // Initialize default demo parameters
    
    //double freq = 30.0;
    //size_t width = 320;
    //size_t height = 240;
    
    std::string topic("/iris/image");
    
    
    //https://github.com/ros2/demos/blob/master/composition/src/manual_composition.cpp
    //node handle
    auto const nh_ = std::make_shared<rclcpp::Node>("camera");
    auto const priv_nh_ = std::make_shared<rclcpp::Node>("_");
    
    auto camera_node = new CameraNode(nh_, priv_nh_);
    
    //rclcpp::spin(nh_);
    //rclcpp::spin(privnh_);
    camera_node->Start();
    // Force flush of the stdout buffer.
    // This ensures a correct sync of all prints
    // even when executed simultaneously within a launch file.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    
    // Create a node.
    //auto node = std::make_shared<Camera>(topic, width, height, freq);
    exec.add_node(nh_);
    exec.add_node(priv_nh_);

    exec.spin();
    //rclcpp::spin(node);
    //rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}
