"use strict";
"require view";
"require rpc";
"require form";
"require uci";
"require ui";
"require poll";

//////////////////////////
// defines
//////////////////////////

const ROLE_DISABLED = 0;
const ROLE_CONTROLLER = 1;
const ROLE_AGENT = 2;
const RADIO_BAND_ALL = 0;
const RADIO_BAND_2G = 1;
const RADIO_BAND_5G = 2;

let vars = {};
vars.channelscanband = RADIO_BAND_ALL;

////////////////////////////
// rpc calls
////////////////////////////
let getChannelScanReport = rpc.declare({
  object: "rtkmap",
  method: "getChannelScanReport",
  params: ["band"],
  expect: { res: "" },
});

let delChannelScanReport = rpc.declare({
  object: "rtkmap",
  method: "delChannelScanReport",
  params: ["band"],
});

let setChannel = rpc.declare({
  object: "rtkmap",
  method: "setChannel",
  params: ["band", "channel"],
  expect: { result: false },
});

let channelScan = rpc.declare({
  object: "rtkmap",
  method: "channelScan",
  params: ["band"],
});


//////////////////////////
// helper functions
//////////////////////////
let getAllChannelScanReport = async function () {
  let res = [];
  for (let i = 0; i < 2; i++) {
    res.push(await getChannelScanReport(i));
  }
  return res;
};

let delAllChannelScanReport = async function () {
  for (let i = 0; i < 2; i++) {
    await delChannelScanReport(i);
  }
};

////////////////////////////
// helper functions: parsers
////////////////////////////


let parseChannelScanReport = function (s) {
  let calcScore = function (x) {
    let cu_weight = 0,
      noise_weight = 0,
      neighbor_weight = 100;
    let res = 0;
    res += ((255 - x.utilization * 100) / 255) * cu_weight;
    res += (100 - x.noise) * noise_weight;
    res += (100 - x.neighbor_nr) * neighbor_weight;
    res /= cu_weight + noise_weight + neighbor_weight;
    return Math.floor(res);
  };
  if (!s) return [];
  let arr = s.split("\n");
  let f = (i) => parseInt(arr[i].split(" ")[1], 10);
  let m = {};
  for (let i = 0; i < arr.length; i += 6) {
    let chn = f(i + 1);
    if (m[chn] === undefined) {
      m[chn] = {
        op_class: [],
        channel: f(i + 1),
        scan_status: f(i + 2),
        utilization: f(i + 3),
        noise: f(i + 4),
        neighbor_nr: f(i + 5),
      };
    }
    let x = m[chn];
    x.op_class.push(f(i));
    x.score = calcScore(x);
  }
  return Object.values(m);
};

/////////////////////////////////
// helper functions: renderers, handlers
/////////////////////////////////

let confirm1 = (deadline) => {
  let tt;
  let ts = Date.now();

  ui.changes.displayStatus("notice");
  let call = () => {
    ui.changes.setIndicator(0);
    ui.changes.displayStatus(
      "notice",
      E("p", _("Configuration changes applied."))
    );

    window.clearTimeout(tt);
    window.setTimeout(function () {
      //ui.changes.displayStatus(false);
      window.location = window.location.href.split("#")[0];
    }, L.env.apply_display * 1000);
  };
  let tick = () => {
    var now = Date.now();

    ui.changes.displayStatus(
      "notice spinning",
      E(
        "p",
        _("Applying configuration changes… %ds").format(
          Math.max(Math.floor((deadline - Date.now()) / 1000), 0)
        )
      )
    );

    if (now >= deadline) return;

    tt = window.setTimeout(tick, 1000 - (now - ts));
    ts = now;
  };
  tick();
  return call;
};

let chnScanCtrlsChange = function () {
  let ctrls = document.getElementById("channelscanctrls");
  if (ctrls) {
    ctrls.innerHTML = "";
    ctrls.appendChild(
      E(
        "div",
        {
          class: "cbi-value",
        },
        [
          E(
            "label",
            {
              class: "cbi-value-title",
            },
            [_("Channel Scan")]
          ),
          E(
            "div",
            {
              class: "cbi-value-field",
            },
            E(
              "button",
              {
                class: "cbi-button cbi-button-apply",
                click: ui.createHandlerFn(this, async function () {
                  vars["channel_0"] && (await setChannel(0, vars["channel_0"]));
                  vars["channel_1"] && (await setChannel(1, vars["channel_1"]));
                  let call = confirm1(Date.now() + L.env.apply_rollback * 1000);
                  setTimeout(call, 5000);
                }),
              },
              [_("Select Best Channel")]
            )
          ),
        ]
      )
    );
  }
};

let handleRescanChannel = async function () {
  await delAllChannelScanReport();
  let tbl = document.getElementById("chnScanTbl");
  if (tbl == undefined) return;
  cbi_update_table(tbl, [], E("em", _("No entries available")));
  await channelScan(vars.channelscanband);
  return new Promise((res, rej) => {
    let f = async () => {
      let [x0, x1] = await getAllChannelScanReport();
      //console.log(`x0=${x0} x1=${x1}`);
      if (x0 == "" && x1 == "") return;
      renderChannelScan(tbl, x0, x1, true);
      chnScanCtrlsChange();
      poll.remove(f);
      res();
    };
    poll.add(f, 5);
  });
};


/////////////////////////////////
// renderers
/////////////////////////////////

let renderChannelScan = function (tbl, x0, x1, selectable) {
  let genRows = function (chnReport, radioname) {
    let genChkbox = function (name, val, selected = false) {
      return E("div", { class: "cbi-radio" }, [
        E("input", {
          type: "radio",
          class: "cbi-input-radio",
          checked: radioname !== undefined && selected ? "" : null,
          style: "-webkit-appearance: radio;",
          disabled: radioname === undefined ? "" : null,
          name: name,
          change: () => {
            vars[`channel_${radioname}`] = val;
          },
          value: val,
        }),
        E("label"),
      ]);
    };

    return chnReport.map((x, i) => {
      return [
        x.channel,
        x.op_class.join(","),
        x.score,
        genChkbox(radioname, x.channel, i === 0),
      ];
    });
  };

  let y0 = parseChannelScanReport(x0).sort(
    (a, b) => b.score - a.score ?? a.channel - b.channel
  );
  let y1 = parseChannelScanReport(x1).sort(
    (a, b) => b.score - a.score ?? a.channel - b.channel
  );
  if (y0.length > 0) vars["channel_0"] = y0[0].channel;
  if (y1.length > 0) vars["channel_1"] = y1[0].channel;
  cbi_update_table(
    tbl,
    [
      ...genRows(y0, selectable ? 0 : undefined),
      ...genRows(y1, selectable ? 1 : undefined),
    ],
    E("em", _("No entries available"))
  );
};

let refreshChannelScanTbl = async function (tbl) {
  if (tbl == undefined) tbl = document.getElementById("chnScanTbl");
  if (tbl == undefined) return;
  let [x0, x1] = await getAllChannelScanReport();
  if (x0 == "" || x1 == "") {
    cbi_update_table(tbl, [], E("em", _("No entries available")));
  } else {
    renderChannelScan(tbl, x0, x1, false);
  }
};

let genChannelScanTbl = function () {
  let tbl = E("table", { class: "table", id: "chnScanTbl" }, [
    E("tr", { class: "tr table-titles" }, [
      E("th", { class: "th" }, [_("Channel")]),
      E("th", { class: "th" }, [_("Op Class")]),
      E("th", { class: "th" }, [_("Score")]),
      E("th", { class: "th" }, [_("Action")]),
    ]),
  ]);
  setTimeout(() => refreshChannelScanTbl(tbl));
  return tbl;
};

let genChannelScanControls = function () {
  return E("div", { id: "channelscanctrls" }, [
    E(
      "div",
      {
        class: "cbi-value",
      },
      [
        E(
          "label",
          {
            class: "cbi-value-title",
          },
          [_("Channel Scan")]
        ),
        E(
          "div",
          {
            class: "cbi-value-field",
          },
          [
            E(
              "div",
              {
                style:
                  "width:100%; overflow: hidden; margin:5px auto;display:flex;flex-direction: row;",
              },
              [
                E("span", {}, _("5G")),
                E(
                  "div",
                  { class: "cbi-radio", style: "margin:auto 15px auto 4px;" },
                  [
                    E("input", {
                      type: "radio",
                      class: "cbi-input-radio",
                      style: "-webkit-appearance: radio;",
                      name: "channelscanband",
                      checked: vars.channelscanband === RADIO_BAND_5G,
                      change: () => {
                        vars.channelscanband = RADIO_BAND_5G;
                      },
                      value: "1",
                    }),
                    E("label"),
                  ]
                ),
                E("span", {}, _("2G")),
                E(
                  "div",
                  { class: "cbi-radio", style: "margin:auto 15px auto 4px;" },
                  [
                    E("input", {
                      type: "radio",
                      class: "cbi-input-radio",
                      style: "-webkit-appearance: radio;",
                      name: "channelscanband",
                      checked: vars.channelscanband === RADIO_BAND_2G,
                      change: () => {
                        vars.channelscanband = RADIO_BAND_2G;
                      },
                      value: "1",
                    }),
                    E("label"),
                  ]
                ),
                E("span", {}, _("All")),
                E(
                  "div",
                  { class: "cbi-radio", style: "margin:auto auto auto 4px;" },
                  [
                    E("input", {
                      type: "radio",
                      class: "cbi-input-radio",
                      style: "-webkit-appearance: radio;",
                      name: "channelscanband",
                      checked: vars.channelscanband === RADIO_BAND_ALL,
                      change: () => {
                        vars.channelscanband = RADIO_BAND_ALL;
                      },
                      value: "1",
                    }),
                    E("label"),
                  ]
                ),
              ]
            ),
            E(
              "button",
              {
                class: "cbi-button cbi-button-edit",
                click: ui.createHandlerFn(this, function () {
                  return handleRescanChannel();
                }),
              },
              [_("Start Channel Scan")]
            ),
          ]
        ),
      ]
    ),
  ]);
};

return view.extend({
  load: async function () {
    await uci.changes();
    await uci.load("rtkmap");
  },

  render: async function () {
    let role = uci.get("rtkmap", "map", "controller");
    if (role != ROLE_CONTROLLER) return E("h3", _("Channel Scan function is available in Controller mode only."));

    return E("div", {}, [genChannelScanControls(), genChannelScanTbl()]);
  },
});
