(function() {
  var params = new Params();

  function mutate() {
    params.mutate();
    updateUI();
    play();
  }

  function randomize() {
    // TODO
    updateUI();
    play();
  }

  function play() {
    var sound = new SoundEffect(params).generate();
    sound.play();
  }

  function serialize() {
    $("#serialized").val(JSON.stringify(params, null, 2));
  }

  function deserialize() {
    var data = JSON.parse($("textarea").val());
    params = Params.fromJSON(data);

    play();
    updateUI();
  }

  function disenable() {
    var duty = params.waveform_type === WAVEFORM_PULSE;
    $("#pulse_width").prop('disabled', !duty);
    $("#pulse_width_sweep_time").prop('disabled', !duty);
  }

  function updateUI() {
    $.each(params, function (param, value) {
      if (param === 'waveform_type') {
        $("input:radio[name=waveform_type]").eq(value).prop('checked', true);
      } else {
        $("input[name=" + param + "]").val(value);
      }
    });
    disenable();
  }

  $(function() {
    $("input[name=waveform_type]").on('change input', function (event) {
      params.waveform_type = parseInt(event.target.value, 10);
      disenable();
      serialize();
      play();
    });

    $("input[type=range].psg-value").on('change', function (event) {
      params[event.target.id] = parseInt(event.target.value, 10);
      serialize();
      play();
    });

    $("input[type=range].psg-value-float").on('change', function (event) {
      params[event.target.id] = parseFloat(event.target.value);
      serialize();
      play();
    });

    $('#button-play').on('click', play);
    $('#button-deserialize').on('click', deserialize);
    $('#button-mutate').on('click', mutate);
    $('#button-randomize').on('click', randomize);

    updateUI();
    serialize();
  });
})();
