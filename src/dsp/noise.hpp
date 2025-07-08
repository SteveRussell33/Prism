#pragma once

#include <random>
// #ifdef ARCH_WIN
// #include <ctime>
// #endif

namespace bogaudio {
	namespace dsp {

		struct Generator {
			float _current = 0.0;

			Generator() {}
			virtual ~Generator() {}

			float current() {
				return _current;
			}

			float next() {
				return _current = _next();
			}

			virtual float _next() = 0;
			
		};

		class Seeds {
		private:
			
			std::mt19937 _generator;
			
			Seeds() {
			// #ifdef ARCH_WIN
			// 	_generator.seed(time(0));
			// #else				
				std::random_device rd;
				_generator.seed(rd());
			// #endif
			}

			unsigned int _next() {
				return _generator();
			}

		public:
			Seeds(const Seeds&) = delete;
			void operator=(const Seeds&) = delete;
			
			static Seeds& getInstance() {
				static Seeds instance;
				return instance;
			}
			
			static unsigned int next() {
				return getInstance()._next();
			};
		};

		struct NoiseGenerator : Generator {
			std::minstd_rand _generator; // one of the faster options.
			NoiseGenerator() : _generator(Seeds::next()) {}
		};

		struct WhiteNoiseGenerator : NoiseGenerator {
			std::uniform_real_distribution<float> _uniform;

			WhiteNoiseGenerator() : _uniform(-1.0, 1.0) {}

			virtual float _next() override {
				return _uniform(_generator);
			}
		};

		template<typename G>
		struct BasePinkNoiseGenerator : NoiseGenerator {
			static const int _n = 6;
			G _g;
			G _gs[_n];
			uint32_t _count = _g.next();

			virtual float _next() override {
				// See: http://www.firstpr.com.au/dsp/pink-noise/
				float sum = _g.next();
				for (int i = 0, bit = 1; i < _n; ++i, bit <<= 1) {
					if (_count & bit) {
						sum += _gs[i].next();
					}
					else {
						sum += _gs[i].current();
					}
				}
				++_count;
				return sum / (float)(_n + 1);
			}
		};

		struct PinkNoiseGenerator : BasePinkNoiseGenerator<WhiteNoiseGenerator> {};

		struct RedNoiseGenerator : BasePinkNoiseGenerator<PinkNoiseGenerator> {};

		struct GaussianNoiseGenerator : NoiseGenerator {
			std::normal_distribution<float> _normal;

			GaussianNoiseGenerator() : _normal(0, 1.0) {}

			virtual float _next() override {
				return _normal(_generator);
			}
		};

		struct FastBrownNoiseGenerator : WhiteNoiseGenerator {
			constexpr static float c0 = 0.0688f;
			constexpr static float c1 = (1. - c0) * 0.984f;

			float _state = 0;

			float _next() override {
				auto r = _uniform(_generator);
				_state = _state * c1 + r * c0;
				return _state;
			}
		};

		class PinkNoiseGenerator2 {
		public:
		    PinkNoiseGenerator2() : rng(std::random_device{}()), white_noise_dist(-1.0, 1.0) {
		        // Initialize the lowpass filter coefficients
		        /*for (size_t i = 0; i < NUM_COEFFICIENTS; ++i) {
		            coefficients[i] = 1.0 / (i + 1);
		        }*/
		    }

		    float next() {
		        // Sum multiple white noise samples to create pink noise
		        float white_noise = white_noise_dist(rng);
		        float pink_noise = 0.f;

		        for (size_t i = 0; i < NUM_COEFFICIENTS; ++i) {
		            pink_noise += coefficients[i] * white_noise; // cppcheck-suppress useStlAlgorithm
		        }

		        return pink_noise;
		    }

		private:
		    static const size_t NUM_COEFFICIENTS = 8;
		    // std::array<double, NUM_COEFFICIENTS> coefficients;
		    static constexpr std::array<float, NUM_COEFFICIENTS> coefficients {
		        1.0, 0.5, 0.333333, 0.25, 0.2, 0.166667, 0.142857, 0.125
		    };
		    std::minstd_rand rng;
		    std::uniform_real_distribution<float> white_noise_dist;
		};

		class PinkNoiseGenerator3 {
		private:
		    std::minstd_rand gen_;
		    std::uniform_real_distribution<double> uniform_u_ {0.0, 1.0};  // For probability selection
		    std::uniform_real_distribution<double> uniform_v_ {-1.0, 1.0}; // For value generation
		    
		    // Generator state
		    double accum_ {};
		    std::array<double, 5> contrib_ {};
		    
		    // Fixed coefficients (from Trammell's floating-point version)
		    static constexpr std::array<double, 5> A_ = {
		        0.4299,  // Weight for generator 0
		        0.3897,  // Weight for generator 1
		        0.3278,  // Weight for generator 2
		        0.3746,  // Weight for generator 3
		        0.4796   // Weight for generator 4
		    };
		    
		    // Cumulative probabilities for generator updates
		    static constexpr std::array<double, 5> P_ = {
		        0.6820,  // Cumulative probability for generator 0
		        0.8517,  // Cumulative probability for generator 1
		        0.9010,  // Cumulative probability for generator 2
		        0.9139,  // Cumulative probability for generator 3
		        0.9165   // Cumulative probability for generator 4
		    };

		public:
		    PinkNoiseGenerator3() {
		        std::random_device rd;
		        gen_.seed(rd());
		    }

		    double generate() {
		        const double u = uniform_u_(gen_); // Select generator
		        const double v = uniform_v_(gen_); // Uniform random value
		        
		        // Update a generator based on cumulative probabilities
		        if (u < P_[0]) {
		            accum_ -= contrib_[0];
		            contrib_[0] = v * A_[0];
		            accum_ += contrib_[0];
		        } else if (u < P_[1]) {
		            accum_ -= contrib_[1];
		            contrib_[1] = v * A_[1];
		            accum_ += contrib_[1];
		        } else if (u < P_[2]) {
		            accum_ -= contrib_[2];
		            contrib_[2] = v * A_[2];
		            accum_ += contrib_[2];
		        } else if (u < P_[3]) {
		            accum_ -= contrib_[3];
		            contrib_[3] = v * A_[3];
		            accum_ += contrib_[3];
		        } else if (u < P_[4]) {
		            accum_ -= contrib_[4];
		            contrib_[4] = v * A_[4];
		            accum_ += contrib_[4];
		        }
		        
		        return accum_; // Output the combined signal
		    }
		};
	} // namespace dsp
} // namespace bogaudio

namespace PaulKellet::dsp {
    class PinkNoise {
    public:
        PinkNoise () : rd {}, mt { rd() }, dist{ -1.0, 1.0 } {
            f0 = f1 = f2 = f3 = f4 = f5 = f6 = 0.f;
        };

        float stepValue () {
            float white = dist(mt);
            // Paul Kellet's refined method, accurate to within +/-0.05dB above 9.2Hz (44.1K sampling rate)
            f0 = 0.99886f * f0 + 0.0555179f * white;
            f1 = 0.99332f * f1 + 0.0750759f * white;
            f2 = 0.96900f * f2 + 0.1538520f * white;
            f3 = 0.86650f * f3 + 0.3104856f * white;
            f4 = 0.55000f * f4 + 0.5329522f * white;
            f5 = -0.7616 * f5 - 0.0168980f * white;
            float pink = f0 + f1 + f2 + f3 + f4 + f5 + f6 + white * 0.5362f;
            // pink *= 0.55f;
            f6 = white * 0.115926f;

            // Paul Kellet's economy method, accurate to within +/-0.5dB above 9.2Hz (44.1K sampling rate)
            // b0 = 0.99765 * b0 + white * 0.0990460;
            // b1 = 0.96300 * b1 + white * 0.2965164;
            // b2 = 0.57000 * b2 + white * 1.0526913;
            // pink = b0 + b1 + b2 + white * 0.1848;

            return pink;
        }

    private:
        std::random_device rd;
        std::mt19937 mt;
        std::uniform_real_distribution<double> dist;
        float f0, f1, f2, f3, f4, f5, f6;
    };
}

namespace tfdsp {
    class WhiteNoiseSource {
    private:
        std::random_device _seed{};
        std::minstd_rand _rng;
        std::normal_distribution<float> _gaussian{0.0, 1.0};
    public:
        WhiteNoiseSource() : _rng(_seed()) {}

        float step() { return _gaussian(_rng); }
    };

    class PinkNoiseSource {
    private:
        WhiteNoiseSource _white{};
        std::array<float, 4> _x{};
        std::array<float, 4> _y{};
        std::array<float, 4> _a{{ 1.0f, -2.494956002f, 2.017265875f, -0.522189400f }};
        std::array<float, 4> _b{{ 0.049922035, -0.095993537, 0.050612699, -0.004408786 }};
    public:
        PinkNoiseSource() {}

        float Filter3dbPerOctave(float x) {
            _x[0] = x;
            auto y = _b[0] * _x[0];
            for(std::size_t i = 1; i < 4; ++i)
                y += _b[i] * _x[i] - _a[i] * _y[i];

            _y[0] = y;
            for(std::size_t i = 3; i > 0; --i) {
                _x[i] = _x[i - 1];
                _y[i] = _y[i - 1];
            }
            return y;
        }

        float step() {
            auto x = _white.step();
            return Filter3dbPerOctave(x);
        }
    };
}

