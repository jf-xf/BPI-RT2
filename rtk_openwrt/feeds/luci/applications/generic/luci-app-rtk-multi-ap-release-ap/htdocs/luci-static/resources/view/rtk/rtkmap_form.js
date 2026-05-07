"use strict";
"require view";
"require rpc";
"require form";
"require uci";
"require ui";


//////////////////////////
// defines
//////////////////////////

const ROLE_DISABLED = 0;
const ROLE_CONTROLLER = 1;
const ROLE_AGENT = 2;
const BAND_ETH = -1;

////////////////////////////
// rpc calls
////////////////////////////

let triggerWPS = rpc.declare({
  object: "rtkmap",
  method: "triggerWPS",
});

let initBhIntf = rpc.declare({
  object: "rtkmap",
  method: "initBhIntf",
});

let restartMap = rpc.declare({
  object: "rtkmap",
  method: "restart",
});


/////////////////////////////////
// renderers
/////////////////////////////////

let genGeneralSettingsView = async function (role, band) {
  let m, s, o;
  m = new form.Map("rtkmap");
  s = m.section(form.TypedSection, "rtk_map", null);
  s.anonymous = true;
  s.redraw = function () {
    return this.load().then(L.bind(this.render, this));
  };
  o = s.option(
    form.ListValue,
    "controller",
    _("Device Role"),
    _("Select Device Role")
  );
  o.placeholder = "placeholder";
  o.value("0", "Disabled");
  o.value("1", "Controller");
  o.value("2", "Agent");
  o.rmempty = false;
  o.editable = true;
  o.onchange = function (ev, section_id, value) {
    s.parse().then(function () { s.redraw(); });
    if (value == '2') {
      ui.showModal(_('Note'), [
        E('p', _('Please enable DHCP client mode!')),
        E('div', {'class': 'right'}, [
          E('button', {
              'class': 'btn',
              'click': ui.hideModal
          }, _('Dismiss'))
        ])
      ]);
    }
  }
  o = s.option(
    form.Value,
    "device_name",
    _("Device Name"),
    _("Input for the device name")
  );
  o.validate = function (section_id, val) {
    if (val == "") return _("must set device name");
    if (val.indexOf(" ") !== -1) return _("no spaces allowed!");
    if (val.indexOf(";") !== -1) return _("';' not allowed!");
    return true;
  };
  o.depends("controller", "1");
  o.depends("controller", "2");
  //o.placeholder = "easymesh_rtk";
  o.rmempty = false;
  o.retain = true;
  o.datatype = "maxlength(32)";

  o = s.option(form.ListValue, "bhband", _("Band"), _("Select Band"));
  o.value("-1", "ETH");
  o.value("0", "2G");
  o.value("1", "5G");
  o.depends("controller", "1");
  o.depends("controller", "2");
  o.rmempty = false;
  o.editable = true;
  o.retain = true;

  if (role == ROLE_DISABLED) return m.render();
  if (band == BAND_ETH) return m.render();

  o = s.option(form.DummyValue, "_wps", _("WPS"), _("WPS button"));
  o.depends("controller", "1");
  o.depends("controller", "2");
  o.cfgvalue = () =>
    E(
      "button",
      {
        class: "cbi-button cbi-button-apply",
        click: ui.createHandlerFn(this, function () {
          return triggerWPS();
        }),
      },
      [_("Start WPS")]
    );

  return m.render();
};


return view.extend({
  load: async function () {
    await uci.changes();
    await uci.load("rtkmap");
  },

  render: async function () {
    let role = uci.get("rtkmap", "map", "controller");
    let band = uci.get("rtkmap", "map", "bhband");
    return genGeneralSettingsView(role, band);
  },

  handleSaveApply: function(ev, mode) {
    return this.handleSave(ev).then(function() {
      classes.ui.changes.apply(mode == '0');
      let init_bh = false;
      for (let chg of (classes.ui.changes.changes?.rtkmap ?? [])) {
        if (chg[2] !== "controller" && chg[2] !== "bhband") continue;
        init_bh = true;
      }
      if (init_bh) initBhIntf();
    });
  },
});
