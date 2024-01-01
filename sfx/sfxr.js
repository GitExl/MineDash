// Heavily modified from https://github.com/chr15m/jsfxr

var WAVEFORM_PULSE = 0;
var WAVEFORM_SAWTOOTH = 1;
var WAVEFORM_TRIANGLE = 2;
var WAVEFORM_NOISE = 3;

var VOLUME_LUT = [
  0,                                  1,  1,  1,
  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,
  4,  4,  4,  4,  5,  5,  5,  6,  6,  7,  7,  7,
  8,  8,  9,  9, 10, 11, 11, 12, 13, 14, 14, 15,
 16, 17, 18, 19, 21, 22, 23, 25, 26, 28, 29, 31,
 33, 35, 37, 39, 42, 44, 47, 50, 52, 56, 59, 63,
];


(function() {
  function frnd(range) {
    return Math.floor(Math.random() * range);
  }

  function rnd(max) {
    return Math.floor(Math.random() * (max + 1));
  }

  function Params() {
    this.waveform_type = WAVEFORM_PULSE;

    // ASD volume envelope.
    this.volume_attack_len = 0;
    this.volume_sustain_len = 30;
    this.volume_decay_len = 15;

    // Tone.
    this.frequency = 200;
    this.frequency_sweep_step = 0;

    // Pulse wave pulse width.
    this.pulse_width = 63;
    this.pulse_width_sweep_time = 0;

    this.volume = 0.2;
  }

  Params.fromJSON = function(data) {
    var params = new Params();
    for (var key in data) {
      if (!data.hasOwnProperty(key)) {
        continue;
      }
      params[key] = data[key];
    }
    return params;
  }

  Params.prototype.mutate = function () {
    if (rnd(1)) this.volume_attack_len += frnd(20) - 10;
    if (rnd(1)) this.volume_sustain_len += frnd(20) - 10;
    if (rnd(1)) this.volume_decay_len += frnd(20) - 10;

    if (rnd(1)) this.frequency += frnd(1000) - 500;
    if (rnd(1)) this.frequency_sweep_step += frnd(6) - 3;

    if (rnd(1)) this.pulse_width += frnd(4) - 2;
    if (rnd(1)) this.pulse_width_sweep_time += frnd(4) - 2;
  }

  function SoundEffect(ps) {
    if (typeof(ps) == "string") {
      var PARAMS = new Params();
      if (ps.indexOf("#") == 0) {
        ps = ps.slice(1);
      }
    }
    this.init(ps);
  }

  SoundEffect.prototype.init = function (ps) {
    this.parameters = ps;

    this.frequency = Math.floor(ps.frequency / (48828.125 / Math.pow(2, 17)));
    this.frequencySweepStep = Math.floor(ps.frequency_sweep_step / (48828.125 / Math.pow(2, 17)));

    this.dutyCycle = ps.pulse_width;
    this.waveShape = ps.waveform_type;

    this.envelopeLength = [
      ps.volume_attack_len,
      ps.volume_sustain_len,
      ps.volume_decay_len
    ];

    this.sampleRate = 48000;
    this.volume = ps.volume;
  }

  SoundEffect.prototype.getRawBuffer = function () {
    var envelopeStage = 0;
    var envelopeElapsed = 0;
    var envelope;
    var envelopeF;

    var frame = 0;
    var phase = 0;
    var newPhase;

    var noiseval = 0;
    var noiseState = 1;

    var sample;
    var sampleL;
    var sampleIndex = 0;
    var sampleCount = (this.envelopeLength[0] + this.envelopeLength[1] + this.envelopeLength[2]) * 800;
    var data = new Float32Array(sampleCount);
    var j;

    while(1) {

      // Frequency slide.
      this.frequency += this.frequencySweepStep;
      this.frequency = Math.min(0xFFFF, Math.max(20, this.frequency));

      // Square wave duty cycle.
      if (!(frame % this.parameters.pulse_width_sweep_time)) {
        this.dutyCycle += Math.sign(this.parameters.pulse_width_sweep_time);
        this.dutyCycle = Math.min(63, Math.max(0, this.dutyCycle));
      }

      // Volume envelope.
      if (++envelopeElapsed > this.envelopeLength[envelopeStage]) {
        envelopeElapsed = 0;
        if (++envelopeStage > 2) {
          break;
        }
      }

      envelopeF = envelopeElapsed / this.envelopeLength[envelopeStage];
      // Attack
      if (envelopeStage === 0) {
        envelope = envelopeF;
      // Sustain
      } else if (envelopeStage === 1) {
        envelope = 1.0 + (1.0 - envelopeF) * 2;
      // Decay
      } else {
        envelope = 1.0 - envelopeF;
      }

      // Generate one frame's worth of new samples (48000Hz / 60).
      for (j = 0; j < 800; j++) {
        noiseState = (noiseState << 1) | (((noiseState >> 1) ^ (noiseState >> 2) ^ (noiseState >> 4) ^ (noiseState >> 15)) & 1);

        // Move phase.
        newPhase = (phase + this.frequency) & 0x1FFFF;
        if ((phase & 0x10000) !== (newPhase & 0x10000)) {
          noiseval = noiseState & 0x3F;
        }
        phase = newPhase;

        // Waveform shape.
        switch (this.waveShape) {
          case WAVEFORM_PULSE: sample = ((phase >> 10) > this.dutyCycle) ? 0 : 0x3F; break;
          case WAVEFORM_SAWTOOTH: sample = phase >> 11; break;
          case WAVEFORM_TRIANGLE: sample = (phase & 0x10000) ? (~(phase >> 10) & 0x3F) : ((phase >> 10) & 0x3F); break;
          case WAVEFORM_NOISE: sample = noiseval; break;
        }
        sample = (sample & 0x3F) / 63;

        // Apply volume.
        sample *= envelope;
        sample *= this.volume;

        sampleL = VOLUME_LUT[Math.floor(sample * 63)] / 63.0;
        data[sampleIndex++] = (sampleL * 2) - 1.0;
      }

      frame++;
    }

    return data;
  }

  SoundEffect.prototype.generate = function() {
    return _sfxr_getAudioFn(this.getRawBuffer());
  }

  var _actx = null;
  var _sfxr_getAudioFn = function(buffer) {
    if (!_actx) {
      _actx = new AudioContext();
      if (!_actx) {
        return {};
      }
    }

    var audioBuffer = _actx.createBuffer(1, buffer.length, 48000);
    audioBuffer.copyToChannel(buffer, 0);

    return {
      play: function() {
        var bufferSource = _actx.createBufferSource();
        bufferSource.buffer = audioBuffer;

        var gainNode = _actx.createGain()
        gainNode.gain.value = 0.25;
        gainNode.connect(_actx.destination)
        bufferSource.connect(gainNode);

        bufferSource.start();
      }
    };
  };

  window.SoundEffect = SoundEffect;
  window.Params = Params;
})();
