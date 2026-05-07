"use strict";
"require view";
"require rpc";
"require form";
"require uci";
"require ui";
"require poll";
"require dom";

//////////////////////////
// defines
//////////////////////////

const ROLE_DISABLED = 0;
const ROLE_CONTROLLER = 1;
const ROLE_AGENT = 2;

const COLOR_1_1 = "#d395fc";
const COLOR_1_2 = "#f0a30a";
const COLOR_1_3 = "#3bb35b";
const COLOR_2_1 = "#00858a";
const COLOR_2_2 = "#08f4fc";
const COLOR_2_3 = "#001b1c";

let vars = {};

let sample1 = `{
  "device_name":"cc",
  "ip_addr":"192.168.1.1",
  "mac_address":"00a16c398001",
  "lan_port_info":[
     {
        "Non_1905_neighbour_mac_address":"000ec6b893af"
     }
  ],
  "station_info":[
    {
      "station_mac": "086266641f4b",
      "station_rssi": "44",
      "station_connected_band": "5G",
      "station_downlink": "260",
      "station_uplink": "292",
      "last_data_downlink_rate": "0",
      "last_data_uplink_rate": "0",
      "bss_transition_support": "0"
    }
  ],
  "neighbor_devices":[
     {
        "neighbor_mac":"12981f007c01",
        "neighbor_name":"easymesh_rtk",
        "neighbor_rssi":"-1",
        "neighbor_band":"ETH"
     }
  ],
  "child_devices":[
     {
        "device_name":"easymesh_rtk",
        "manufacturer":"Realtek",
        "model_name":"RTL8198D",
        "ip_addr":"192.168.1.2",
        "mac_address":"12981f007c01",
        "child_rssi":"-1",
        "child_band":"ETH",
        "lan_port_info":[
           
        ],
        "station_info":[
           
        ],
        "neighbor_devices":[
           
        ],
        "child_devices":[
           
        ]
     }
  ]
}`;

////////////////////////////
// rpc calls
////////////////////////////
let getTopologyJson = rpc.declare({
  object: "rtkmap",
  method: "getTopologyJson",
  expect: { res: "" },
});

//////////////////////////
// helper functions
//////////////////////////
let genHtmlTable = function (data) {
  const t = document.createElement("table");
  let r, c;
  for (let i = 0; i < data.length; i++) {
    r = t.insertRow();
    for (let j = 0; j < data[i].length; j++) {
      c = r.insertCell();
      c.innerText = data[i][j];
    }
  }
  return t;
};

let CSVToArray = function (
  data,
  delimiter = ",",
  dequote = false,
  omitFirstRow = false
) {
  return data
    .slice(omitFirstRow ? data.indexOf("\n") + 1 : 0)
    .split("\n")
    .map((v) =>
      v
        .split(delimiter)
        .map((x) => (dequote ? x.trim().replace(/['"]+/g, "") : x))
    );
};

////////////////////////////
// helper functions: parsers
////////////////////////////

let parseTopologyJson = function (s) {
  let ss = JSON.parse(s);

  let getEdgeColor = function (conntype) {
    switch (conntype) {
      case "5G":
        return COLOR_2_1;
      case "2G":
        return COLOR_2_2;
      case "ETH":
      default:
        return COLOR_2_3;
    }
  };
  let parseSelfDesc = function (dev) {
    return genHtmlTable(
      CSVToArray(
        `Type;Controller
        Device Name;${dev.device_name}
        IP Address;${dev.ip_addr}
        MAC Address;${dev.mac_address}`,
        ";",
        true
      )
    );
  };
  let parseAgtDesc = function (dev) {
    return genHtmlTable(
      CSVToArray(
        `Type;Agent
        Device Name;${dev.device_name}
        Manufacturer;${dev.manufacturer}
        Model name;${dev.model_name}
        IP Address;${dev.ip_addr}
        MAC Address;${dev.mac_address}
        RSSI;${dev.child_rssi}
        Band;${dev.child_band}`,
        ";",
        true
      )
    );
  };
  let parseStaDesc = function (sta) {
    return genHtmlTable(
      CSVToArray(
        `Type;Station
        MAC Address;${sta.station_mac}
        RSSI;${sta.station_rssi}
        Band;${sta.station_connected_band}`,
        ";",
        true
      )
    );
  };

  let nodes = [],
    edges = [];
  let addStas = (ss, parent_node_id) =>
    ss.station_info.forEach((x) => {
      let nodeid = nodes.length + 1;
      nodes.push({
        id: nodeid,
        label: x.station_mac,
        title: parseStaDesc(x),
        group: "sta",
      });
      edges.push({
        from: parent_node_id,
        to: nodeid,
        title: x.station_connected_band,
        color: getEdgeColor(x.station_connected_band),
        width: 2,
      });
    });
  let traverseChild = (ss, parent_node_id) =>
    ss.child_devices.forEach((x) => {
      let nodeid = nodes.length + 1;
      nodes.push({
        id: nodeid,
        label: x.device_name,
        title: parseAgtDesc(x),
        group: "agt",
      });
      edges.push({
        from: parent_node_id,
        to: nodeid,
        title: x.child_band,
        color: getEdgeColor(x.child_band),
        width: 2,
      });
      addStas(x, nodeid);
      traverseChild(x, nodeid);
    });
  nodes.push({
    id: 1,
    label: ss.device_name,
    title: parseSelfDesc(ss),
    group: "self",
  });
  addStas(ss, 1);
  traverseChild(ss, 1);

  let options = {
    groups: {
      self: {
        color: COLOR_1_1,
      },
      agt: {
        color: COLOR_1_2,
      },
      sta: {
        color: COLOR_1_3,
      },
    },
  };
  return [nodes, edges, options];
};

let parseTopologyJson2 = function (s) {
  let ss = JSON.parse(s);
  let devs = [];
  let traverseChild = (ss) =>
    ss.child_devices.forEach((x) => {
      devs.push({ ...x, parent: ss });
      traverseChild(x);
    });

  devs.push(ss);
  traverseChild(ss);
  return devs;
};

/////////////////////////////////
// helper functions: renderers, handlers
/////////////////////////////////
let hideModal = (ev) => {
  let md = dom.parent(ev.target, 'div[aria-modal="true"]');
  if (md) {
    md.style.maxWidth = "";
    md.style.maxHeight = "";
  }

  ui.hideModal();
};

let refreshTopologyJson = async () => {
  let res = await getTopologyJson();
  //console.log(`res=${res}`);
  if (res === vars.lastTopoJson) return null;
  //res = sample1;

  return (vars.lastTopoJson = res);
};

let renderTopologyGraph = (topoJson, container) => {
  if (container == undefined) container = document.getElementById("mynetwork");
  if (container == undefined) return;

  let data = parseTopologyJson(topoJson);
  if (data == undefined) return;
  let [nodes, edges, options] = data;
  nodes = new vis.DataSet(nodes);
  edges = new vis.DataSet(edges);

  if (vars.network) vars.network.destroy();
  vars.network = new vis.Network(container, { nodes, edges }, options);
};

let genTopologyDeviceDetail = (dev) => {
  let neighbourTbl = E("table", { class: "table" }, [
    E("tr", { class: "tr table-titles" }, [
      E("th", { class: "th" }, [_("Name")]),
      E("th", { class: "th" }, [_("MAC")]),
      E("th", { class: "th" }, [_("Band")]),
      E("th", { class: "th" }, [_("RSSI")]),
    ]),
  ]);
  let staTbl = E("table", { class: "table" }, [
    E("tr", { class: "tr table-titles" }, [
      E("th", { class: "th" }, [_("MAC")]),
      E("th", { class: "th" }, [_("Band")]),
      E("th", { class: "th" }, [_("RSSI")]),
    ]),
  ]);
  cbi_update_table(
    neighbourTbl,
    dev.neighbor_devices.map((x) => [
      x.neighbor_name,
      x.neighbor_mac,
      x.neighbor_band,
      x.neighbor_rssi,
    ]),
    E("em", _("No entries available"))
  );
  cbi_update_table(
    staTbl,
    dev.station_info.map((x) => [
      x.station_mac,
      x.station_connected_band,
      x.station_rssi,
    ]),
    E("em", _("No entries available"))
  );
  return E("div", {}, [
    E("h3", {}, _("Neighbouring Devices")),
    neighbourTbl,
    E("h3", {}, _("Associated Stations")),
    staTbl,
  ]);
};

let refreshTopologyTbl = async (container) => {
  let showDetailsModal = (dev) => {
    let md = ui.showModal(_("Device Details"), [
      genTopologyDeviceDetail(dev),
      E("div", { class: "right" }, [
        E(
          "button",
          {
            class: "btn",
            click: hideModal,
          },
          _("Dismiss")
        ),
      ]),
    ]);

    md.style.maxWidth = "90%";
    md.style.maxHeight = "none";
  };
  let _f = (res) => {
    if (res == "") return [];
    let devs = parseTopologyJson2(res);
    if (devs == undefined) return [];
    // X2YxRPfCz
    return devs.map((x) => [
      x.device_name,
      x.ip_addr,
      x.mac_address,
      x.parent?.mac_address ?? "-",
      x.child_band ?? "-",
      E(
        "button",
        {
          class: "cbi-button",
          title: _("Show Details"),
          click: ui.createHandlerFn(this, function () {
            showDetailsModal(x);
          }),
        },
        _("Details")
      ),
    ]);
  };
  if (container == undefined)
    container = document.getElementById("mynetworktbl");
  if (container == undefined) return;
  let res = await refreshTopologyJson();
  if (res === null) return;

  cbi_update_table(container, _f(res), E("em", _("No entries available")));
};

let genTopologyGraph = function () {
  let container = E(
    "div",
    {
      id: "mynetwork",
      style: `min-width: 300px;
     width: auto;
     min-height: 300px;
     height: 70vh;
     border: 1px solid lightgray`,
    },
    [_("No data")]
  );

  let res = E("div", { style: "margin: 10px" }, [
    E(
      "div",
      {
        class: "cbi-value",
      },
      [
        E("span", { style: `color:${COLOR_1_1}` }, "⬤ "),
        E(
          "div",
          {
            style: "margin: 2px 10px",
          },
          _("Controller")
        ),
        E("span", { style: `color:${COLOR_1_2}` }, "⬤ "),
        E(
          "div",
          {
            style: "margin: 2px 10px",
          },
          _("Agent")
        ),
        E("span", { style: `color:${COLOR_1_3}` }, "⬤ "),
        E(
          "div",
          {
            style: "margin: 2px 10px",
          },
          _("Station")
        ),
        E("span", { style: `color:${COLOR_2_1}` }, "■ "),
        E(
          "div",
          {
            style: "margin: 2px 10px",
          },
          _("5G")
        ),
        E("span", { style: `color:${COLOR_2_2}` }, "■ "),
        E(
          "div",
          {
            style: "margin: 2px 10px",
          },
          _("2G")
        ),
        E("span", { style: `color:${COLOR_2_3}` }, "■ "),
        E(
          "div",
          {
            style: "margin: 2px 10px",
          },
          _("ETH")
        ),
      ]
    ),
    container,
  ]);

  setTimeout(() => renderTopologyGraph(vars.lastTopoJson, container));
  return res;
};

let showTopologyGraphModal = function () {
  let refreshBtn = E(
    "button",
    {
      class: "btn",
      click: ui.createHandlerFn(this, async function () {
        let res = await refreshTopologyJson();
        if (res === null) return;
        renderTopologyGraph(res);
      }),
    },
    _("Refresh")
  );
  let dismissBtn = E(
    "button",
    {
      class: "btn",
      click: hideModal,
    },
    _("Dismiss")
  );

  let md = ui.showModal(_("Topology Graph"), [
    genTopologyGraph(),
    E("div", { class: "right" }, [refreshBtn, " ", dismissBtn]),
  ]);

  md.style.maxWidth = "90%";
  md.style.maxHeight = "none";
};

/////////////////////////////////
// renderers
/////////////////////////////////
let genTopologyTbl = function () {
  // X2YxRPfCz
  let tbl = E("table", { class: "table", id: "mynetworktbl" }, [
    E("tr", { class: "tr table-titles" }, [
      E("th", { class: "th" }, [_("Device Name")]),
      E("th", { class: "th" }, [_("Device IP")]),
      E("th", { class: "th" }, [_("Device MAC")]),
      E("th", { class: "th" }, [_("Parent MAC")]),
      E("th", { class: "th" }, [_("Connection Type")]),
      E("th", { class: "th" }, [_("Action")]),
    ]),
  ]);
  let res = E("div", {}, [
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
          [_("Topology Graph")]
        ),
        E(
          "div",
          {
            class: "cbi-value-field",
          },
          [
            E(
              "button",
              {
                class: "cbi-button",
                click: ui.createHandlerFn(this, showTopologyGraphModal),
              },
              [_("Show Graph")]
            ),
          ]
        ),
      ]
    ),
    tbl,
  ]);

  setTimeout(() => refreshTopologyTbl(tbl));
  poll.add(refreshTopologyTbl, 5);
  return res;
};

return view.extend({
  load: async function () {
    await uci.changes();
    await uci.load("rtkmap");
    let role = uci.get("rtkmap", "map", "controller");
    if (role != ROLE_CONTROLLER) return;
    let fr = await fetch("/luci-static/resources/vis.js");
    eval(await fr.text());
  },

  render: async function () {
    let role = uci.get("rtkmap", "map", "controller");
    if (role != ROLE_CONTROLLER) return E("h3", _("Topology function is available in Controller mode only."));
    return genTopologyTbl();
  },
});
