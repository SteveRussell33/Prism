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
		    PinkNoiseGenerator2() {
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
		        
		        return accum_; // The combined signal
		    }
		};
	} // namespace dsp
} // namespace bogaudio

namespace PaulKellet::dsp {
    class PinkNoise {
    public:
        PinkNoise () : rd {}, mt { rd() }, dist{ -1.0, 1.0 } {
            b0 = b1 = b2 = b3 = b4 = b5 = b6 = 0.f;
        };

        float stepValue () {
            float white = dist(mt);
            // Paul Kellet's refined method, accurate to within +/-0.05dB above 9.2Hz (44.1K sampling rate)
            b0 = 0.99886 * b0 + white * 0.0555179;
			b1 = 0.99332 * b1 + white * 0.0750759;
			b2 = 0.96900 * b2 + white * 0.1538520;
			b3 = 0.86650 * b3 + white * 0.3104856;
			b4 = 0.55000 * b4 + white * 0.5329522;
			b5 = -0.7616 * b5 - white * 0.0168980;
            float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
            b6 = white * 0.115926f;
            pink *= 0.67f;

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
        float b0, b1, b2, b3, b4, b5, b6;
    };
}
