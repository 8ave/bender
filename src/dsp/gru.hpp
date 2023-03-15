#pragma once

#include <map>
#include <fstream>
#include <iostream>

#include <variant>
#include <vector>
#include <memory>

// 4127 suppression can be removed for eigen >3.4.0
#ifdef _MSC_VER
  #pragma warning( push )
  // conditional expression is constant
  #pragma warning( disable : 4127 )
#endif
#include <Eigen/Dense>
#ifdef _MSC_VER
  #pragma warning( pop )
#endif


#include "weights.cxx"

namespace dsp {

inline float lerp(float a, float b, float f) {
    return a * (1.0f - f) + b * f;
}

struct ApproximateMath1 {
    static float tanh(float x) {
        return std::tanh(x);
    }

    static float sigmoid(float x) {
        return (tanh(x / 2.0f) + 1.0f) / 2.0f;
    }
};

struct ApproximateMath2 {
    static float tanh(float x) {
        float x2 = x * x;
        float x3 = x2 * x;
        float x5 = x3 * x2;
        float a = x + 0.16489087f * x3 + 0.00985468f * x5;
        return a / sqrtf(1.0f + (a * a));
    }

    static float sigmoid(float x) {
        return (tanh(x/2.0f) + 1.0f) / 2.0f;
    }
};

template<int input_size, int hidden_size, int output_size=1>
struct Gru {

    struct Model {
        using InputToHidden = Eigen::Matrix<float, hidden_size*3, input_size>;
        using HiddenToHidden = Eigen::Matrix<float, hidden_size, hidden_size>;
        using HiddenToHidden3 = Eigen::Matrix<float, hidden_size*3, hidden_size>;
        using HiddenToOutput = Eigen::Matrix<float, output_size, hidden_size>;
        using HiddenBias = Eigen::Matrix<float, hidden_size*3, 1>;
        HiddenToOutput w_h2o;

        InputToHidden w_i;
        HiddenToHidden3 w_h;

        HiddenBias b_i;
        HiddenBias b_h;

        float res1_low;
        float res1_high;
        float res2_low;
        float res2_high;

        Model(int model_index = 0) {
            if (model_index == 0) {
                res1_low = hardcoded_res1_low;
                res1_high = hardcoded_res1_high;
                res2_low = hardcoded_res2_low;
                res2_high = hardcoded_res2_high;

                auto fill = [&](auto& mat, const std::vector<float>& source) {
                    size_t index = 0;
                    for (int i = 0; i < mat.rows(); ++i) {
                        for (int j = 0; j < mat.cols(); ++j) {
                            mat(i, j) = source[index];
                            ++index;
                        }
                    }
                };

                fill(w_h2o, hardcoded_w_h2o);
                fill(w_i, hardcoded_w_i);
                fill(w_h, hardcoded_w_h);
                fill(b_i, hardcoded_b_i);
                fill(b_h, hardcoded_b_h);
            } else {
                res1_low = hardcoded2_res1_low;
                res1_high = hardcoded2_res1_high;
                res2_low = hardcoded2_res2_low;
                res2_high = hardcoded2_res2_high;

                auto fill = [&](auto& mat, const std::vector<float>& source) {
                    size_t index = 0;
                    for (int i = 0; i < mat.rows(); ++i) {
                        for (int j = 0; j < mat.cols(); ++j) {
                            mat(i, j) = source[index];
                            ++index;
                        }
                    }
                };

                fill(w_h2o, hardcoded2_w_h2o);
                fill(w_i, hardcoded2_w_i);
                fill(w_h, hardcoded2_w_h);
                fill(b_i, hardcoded2_b_i);
                fill(b_h, hardcoded2_b_h);
            }

        }

    };

    std::unique_ptr<Model> own_model;

    const Model& model;

    float dc_alpha{0.99f};
    float dc_beta{0.01f};

    float param_smooth_alpha{ 0.99f };
    float param_smooth_beta{ 0.01f };

    float matrix_smooth_alpha{ 0.999f };
    float matrix_smooth_beta{ 0.001f };

    struct State {
        using HiddenToOutputTransposed = Eigen::Matrix<float, hidden_size, 1>;
        using HiddenToHidden = Eigen::Matrix<float, hidden_size, hidden_size>;

        HiddenToOutputTransposed hidden{HiddenToOutputTransposed::Zero()};
        float feedback{ 0.0f };

        HiddenToHidden smoothedBendingMatrix1{ HiddenToHidden::Zero() };
        HiddenToHidden smoothedBendingMatrix2{ HiddenToHidden::Zero() };

        float smooth_res{ 0.0f };
        float smooth_cutoff{ 0.0f };
        float smooth_cutoff2{ 0.0f };
        float dc{ 0.0f };

    };

    State state{};

    Gru() :
        own_model(std::make_unique<Model>()),
        model(*own_model) {

        float samplerate_ = 88200.0f;

        reset(samplerate_);

    }

    template<typename ApproximateMath>
    float process(float input_, float cutoff, float res_,
                  const typename Model::HiddenToHidden& bendingMatrix1,
                  const typename Model::HiddenToHidden& bendingMatrix2,
                  Eigen::Matrix<float, hidden_size, 1>& other_hidden) {
        using namespace Eigen;
        Matrix<float, 4, 1> input;

        constexpr int hs = hidden_size;
        constexpr int hs3 = hidden_size * 3;

        // There are 2 ways to introduce resonance to a filter
        // 1. Train a model with a resonance input parameter
        // 2. Add 1-unit delay feedback

        float res1 = dsp::lerp(model.res1_low, model.res1_high, res_);
        float res2 = dsp::lerp(model.res2_low, model.res2_high, res_);

        cutoff = std::clamp(cutoff, 0.0f, 1.0f);
        res1 = std::clamp(res1, 0.0f, 1.0f);

        state.smooth_res = param_smooth_alpha * state.smooth_res + param_smooth_beta * res1;
        state.smooth_cutoff = param_smooth_alpha * state.smooth_cutoff + param_smooth_beta * cutoff;

        input[0] = input_ + state.feedback * res2;
        input[1] = state.smooth_cutoff;
        input[2] = state.smooth_res;
        if constexpr (input_size >= 4) {
            input[3] = sqrtf(state.smooth_cutoff);
        }

        // vanilla GRU
        // this is a translation of a vectorized python code to Eigen
        // see https://github.com/Maghoumi/JitGRU/blob/master/jit_gru.py#L51 as an example
        // proj_i(seq(fix<0>, fix<hs-1>)) is an analog to proj_i[:hiddenSize-1, :] in python,
        // so all these seq(fix, fix) operations split proj_i and proj_h into 3 equally sized chunks
        // long one-liners should help Eigen emit optimal code

        Matrix<float, hs3, 1> proj_i = model.w_i*input + model.b_i;
        Matrix<float, hs3, 1> proj_h = model.w_h*state.hidden + model.b_h;

        Matrix<float, hs, 1> R = (proj_i(seq(fix<0>, fix<hs-1>)) + proj_h(seq(fix<0>, fix<hs-1>))).unaryExpr(&ApproximateMath::sigmoid);
        Matrix<float, hs, 1> Z = (proj_i(seq(fix<hs>, fix<2*hs-1>)) + proj_h(seq(fix<hs>, fix<2*hs-1>))).unaryExpr(&ApproximateMath::sigmoid);
        Matrix<float, hs, 1> H_tilda = (proj_i(seq(fix<2*hs>, fix<3*hs-1>))
                                        + R.cwiseProduct(proj_h(seq(fix<2*hs>, fix<3*hs-1>)))).unaryExpr(&ApproximateMath::tanh);
        state.hidden = Z.cwiseProduct(state.hidden) + (Matrix<float, hs, 1>::Ones() - Z).cwiseProduct(H_tilda);

        // end of vanilla GRU

        // smoothing the bending matrices They are 64x64, so it's rather slow
        state.smoothedBendingMatrix1 = matrix_smooth_alpha * state.smoothedBendingMatrix1 + matrix_smooth_beta * bendingMatrix1;
        state.smoothedBendingMatrix2 = matrix_smooth_alpha * state.smoothedBendingMatrix2 + matrix_smooth_beta * bendingMatrix2;

        // main idea: rewire signal flow in the feedback path
        state.hidden = state.smoothedBendingMatrix1 * state.hidden;
        state.hidden += state.smoothedBendingMatrix2 * other_hidden;

        // hidden state is expected to be in -1..1 range
        // so after rewiring, apply another tanh to make it more stable
        state.hidden = (state.hidden / 1.5f).unaryExpr(&ApproximateMath::tanh) * 1.5f;

        // NaNs shouldn't appear, but if they do
        // the plugin shouldn't get silent forever
        for (int i = 0; i < hs; ++i) {
            if (!std::isfinite(state.hidden[i])) {
                state.hidden.setZero();
                break;
            }
        }

        // w_h2o maps hidden state to output values and it's another interesting place to tweak
        float output_sample = model.w_h2o * state.hidden;

        // highpass output in case the model generates a slight DC offset
        state.dc = dc_alpha * state.dc + dc_beta * output_sample;

        state.feedback = output_sample - state.dc;
        return state.feedback;
    }

    void reset(float samplerate) {
        state = State();

        dc_alpha = expf(-2.0f * float(M_PI) * 5.0f / samplerate);
        dc_beta = 1.0f - dc_alpha;

        param_smooth_alpha = expf(-2.0f * float(M_PI) * 40.0f / samplerate);//40
        param_smooth_beta = 1.0f - param_smooth_alpha;

        matrix_smooth_alpha = expf(-2.0f * float(M_PI) * 0.4f / samplerate);
        matrix_smooth_beta = 1.0f - matrix_smooth_alpha;
    }

};

extern int dummy_gru;


}
