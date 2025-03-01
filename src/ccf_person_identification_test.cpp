#include <iostream>
#include <ros/package.h>

#include <boost/format.hpp>
#include <opencv2/opencv.hpp>
#include <ccf_person_identification/sobit_person_classifier.hpp>


using namespace ccf_person_classifier;

int main(int argc, char** argv) {
    ros::init(argc, argv, "ccf_person_identification_test");
    ros::NodeHandle nh;
    ros::NodeHandle private_nh("~");

    std::unique_ptr<BodyClassifier> classifier(new BodyClassifier(nh));

    std::string package_path = ros::package::getPath("ccf_person_identification");
    std::string dataset_dir = package_path + "/data/test";

    // train the classifier with the first ten frames, and test it with the rest frames
    for(int i=1; i<=14; i++) {
        std::unordered_map<std::string, cv::Mat> pos, neg1, neg2;
        //pos["body"], neg1["body"], neg2["body"]は画像を読み込み
        pos["body"] = cv::imread((boost::format("%s/p%02d.jpg") % dataset_dir % i).str());
        neg1["body"] = cv::imread((boost::format("%s/n%02d-01.jpg") % dataset_dir % i).str());
        neg2["body"] = cv::imread((boost::format("%s/n%02d-02.jpg") % dataset_dir % i).str());

        if(!pos["body"].data || !neg1["body"].data || !neg2["body"].data) {
            std::cerr << "error : failed to open image!! image_id: " << i << std::endl;
            return 1;
        }

        if(i <= 10) {
            ROS_INFO_STREAM("training");
        } else {
            ROS_INFO_STREAM("testing");
        }
        
        //参考URL：http://opencv.jp/opencv-2svn/cpp/geometric_image_transformations.html
        //サイズ変換を行う
        cv::Mat pos_result;
        cv::resize(pos["body"], pos_result, cv::Size(128, 256));

        cv::Mat neg1_result;
        cv::resize(neg1["body"], neg1_result, cv::Size(128, 256));

        cv::Mat neg2_result;
        cv::resize(neg2["body"], neg2_result, cv::Size(128, 256));
        
        //new：メモリの動的確保
        //InPut, Features：online_classifier.hppのクラス
        //PerosnInout, PersonFeatures：person_classifier.hppのクラス
        Input::Ptr input(new PersonInput());
        Features::Ptr features(new PersonFeatures());
        
        // classifier：body_classifier.hppのextractInputの処理
        // ⇒posに["body"]が含まれている場合、何も処理を加えない
        // ⇒posに["body"]がない場合はなにか処理を加えている
        classifier->extractInput(input, pos);
        //featuresがない or inputがない場合、falseを返す
        //両方あった場合、feature_mapsを生成するような処理をする
        classifier->extractFeatures(features, input);
        //classifier->predict：featuresをもとに信頼度を返す(online_boosting.hpp：predictReal)
        //predict(features)が0.0以上の場合：赤色、0.0以下の場合：青色
        cv::Scalar pos_color = *classifier->predict(features) > 0.0 ? cv::Scalar(0, 0, 255) : cv::Scalar(255, 0, 0);
        // 長方形の描画
        cv::rectangle(pos_result, cv::Point(0, 0), cv::Point(128, 256), pos_color, 5);

        ROS_INFO_STREAM("pos:" << *classifier->predict(features));

        classifier->extractInput(input, neg1);
        classifier->extractFeatures(features, input);
        cv::Scalar neg1_color = *classifier->predict(features) > 0.0 ? cv::Scalar(0, 0, 255) : cv::Scalar(255, 0, 0);
        cv::rectangle(neg1_result, cv::Point(0, 0), cv::Point(128, 256), neg1_color, 5);

        ROS_INFO_STREAM("neg1:" << *classifier->predict(features));

        classifier->extractInput(input, neg2);
        classifier->extractFeatures(features, input);
        cv::Scalar neg2_color = *classifier->predict(features) > 0.0 ? cv::Scalar(0, 0, 255) : cv::Scalar(255, 0, 0);
        cv::rectangle(neg2_result, cv::Point(0, 0), cv::Point(128, 256), neg2_color, 5);

        ROS_INFO_STREAM("neg2:" << *classifier->predict(features));

        std::vector<cv::Mat> results = {pos_result, neg1_result, neg2_result};
        cv::Mat canvas;
        //hconcat：opencvで画像を横に連結する
        //putText：画像内にテキストを挿入(i<=10の場合：training、それ以外はtesting)
        cv::hconcat(results, canvas);
        cv::putText(canvas, i <= 10 ? "training" : "testing", cv::Point(10, 25), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar::all(0), 2);
        cv::putText(canvas, i <= 10 ? "training" : "testing", cv::Point(10, 25), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar::all(255));
        cv::imshow("results", canvas);

        cv::Mat feature_map = classifier->visualize();
        if(feature_map.data) {
            cv::imshow("feature_map", feature_map);
        }
        cv::waitKey(0);

        if(i <= 10)  {
            classifier->extractInput(input, pos);
            classifier->extractFeatures(features, input);
            bool pflag = classifier->update(1.0, features);
            if(!pflag){
                ROS_INFO_STREAM("pflag_failed");
            }
            classifier->extractInput(input, neg1);
            classifier->extractFeatures(features, input);
            classifier->update(-1.0, features);

            // classifier->extractInput(input, neg2);
            // classifier->extractFeatures(features, input);
            // classifier->update(-1.0, features);
        }
    }

    return 0;
}
