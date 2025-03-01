#ifndef SOBIT_PERSON_CLASSIFIER_HPP
#define SOBIT_PERSON_CLASSIFIER_HPP

#include <vector>
#include <ccf_person_identification/online_classifier.hpp>
#include <ccf_person_identification/face/face_classifier.hpp>
#include <ccf_person_identification/body/body_classifier.hpp>


namespace ccf_person_classifier {

class PersonInput : public BodyInput, public FaceInput {
public:
    virtual ~PersonInput() override {}
    using Ptr = std::shared_ptr<PersonInput>;
};

class PersonFeatures : public BodyFeatures, public FaceFeatures {
public:
    virtual ~PersonFeatures() override {}
    using Ptr = std::shared_ptr<PersonFeatures>;
};


class PersonClassifier : public OnlineClassifier {
public:
    PersonClassifier(ros::NodeHandle& nh)
    {
        // 従来から変更した（use_faceのdefaultのbool値をtrue→falseに変更）（2024/8/6 by mukogawa）
        if(nh.param<bool>("use_face", false)) {
            classifiers.push_back(std::make_shared<FaceClassifier>(nh));
        }
        if(nh.param<bool>("use_body", true)) {
            classifiers.push_back(std::make_shared<BodyClassifier>(nh));
        }
    }

    virtual ~PersonClassifier() override {}

    virtual std::string name() const override {
        return "face+body";
    }

    std::vector<std::string> classifierNames() const {
        std::vector<std::string> names;
        for(const auto& classifier: classifiers) {
            names.push_back(classifier->name());
        }

        return names;
    }

    template<typename Classifier>
    std::shared_ptr<Classifier> getClassifier(const std::string& name) const {
        for(const auto& classifier : classifiers) {
            if(name == classifier->name()) {
                return std::dynamic_pointer_cast<Classifier>(classifier);
            }
        }

        return nullptr;
    }

    virtual bool extractInput(Input::Ptr& input, const std::unordered_map<std::string, cv::Mat>& images) override {
        std::vector<bool> extracted(classifiers.size(), false);
        // std::transformは全ての要素(classifiers.begin(), classifiers.end(), extracted.begin())に関数を適用する
        std::transform(classifiers.begin(), classifiers.end(), extracted.begin(),
            [&](const OnlineClassifier::Ptr& classifier) {
                // classifier：body_classifier.hppのextractInputの処理
                // ⇒posに["body"]が含まれている場合、何も処理を加えない
                // ⇒posに["body"]がない場合はなにか処理を加えている
                return classifier->extractInput(input, images);
            }
        );

        return std::any_of(extracted.begin(), extracted.end(), [=](bool b) { return b; });
    }

    virtual bool extractFeatures(Features::Ptr& features, const Input::Ptr& input) override {
        std::vector<bool> extracted(classifiers.size(), false);
        std::transform(classifiers.begin(), classifiers.end(), extracted.begin(),
            [&](const OnlineClassifier::Ptr& classifier) {
                return classifier->extractFeatures(features, input);
            }
        );

        return std::any_of(extracted.begin(), extracted.end(), [=](bool b) { return b; });
    }

    virtual bool update(double label, const Features::Ptr& features) override {
        // 
        std::vector<bool> updated(classifiers.size(), false);
        std::transform(classifiers.begin(), classifiers.end(), updated.begin(),
            [&](const OnlineClassifier::Ptr& classifier) {
                return classifier->update(label, features);
            }
        );

        return std::any_of(updated.begin(), updated.end(), [=](bool b) { return b; });
    }

    virtual boost::optional<double> predict(const Features::Ptr& features) override {
        ROS_ERROR_STREAM("this method must not be called!!");
        abort();
        return boost::none;
    }

    boost::optional<double> predict(const Features::Ptr& features, std::vector<double>& classifier_confidences) {
        std::vector<boost::optional<double>> results(classifiers.size(), false);
        // std::transform⇒https://cpprefjp.github.io/reference/algorithm/transform.html
        // 各classifier（classifiers内の要素）について、
        // classifier->predict(features)が呼び出され、
        // その結果がresultsの対応する位置に格納
        std::transform(classifiers.begin(), classifiers.end(), results.begin(),
            [&](const OnlineClassifier::Ptr& classifier) {
                return classifier->predict(features);
            }
        );
        // もしclassifier_confidencesがなにも入っていない場合
        if(classifier_confidences.empty()) {
            // ROS_INFO("--classifier_confidences empty--");
            // 要素数：results.size()、値：-0.2で初期化を行う
            classifier_confidences.resize(results.size(), -0.2);
        }
        // std::cout << "results.size() :" << results.size() << std::endl;
        // resultsの要素分繰り返す
        for(int i=0; i<results.size(); i++) {
            if(results[i]) {
                classifier_confidences[i] = *results[i];
            }
        }
        // boost::optionalは、値の存在または不在を表現するためのクラス
        // BUG(6/8)：result[0]⇒results[1]にしたらうまくいった
        // その後、従来より少し変更した（2024/8/6 by mukogawa）
        boost::optional<double> aggregated = results[0];
        // if(aggregated)の条件文で、aggregatedが値を持っているかどうかをチェック
        // もしaggregatedが値を持っている場合（存在する場合）、以下の処理が実行
        if(aggregated) {
            // forループは、resultsの2番目の要素（results[1]）から最後の要素まで繰り返す
            for(int i=1; i<results.size(); i++) {
                // boost::optional<double>型のオブジェクトresults[i]が存在する場合、*results[i]をconfに代入
                // 存在しない場合は、代わりにclassifier_confidences[i]が使用
                double conf = results[i] ? *results[i] : classifier_confidences[i];
                // *aggregated += conf;は、aggregatedの値にconfを加算
                // *aggregatedはaggregatedが保持している値を取得するために使用
                // if(i != 0) {
                //     *aggregated += conf;
                // }
                *aggregated += conf;
            }
            // forループが終了すると、*aggregatedの値をresultsの要素数で割る
            // 集約された値の平均が計算
            *aggregated /= results.size();
        }
        return aggregated;
    }

private:
    std::vector<OnlineClassifier::Ptr> classifiers;
};

}

#endif // SOBIT_PERSON_CLASSIFIER_HPP
