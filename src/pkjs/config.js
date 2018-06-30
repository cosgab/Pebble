module.exports = [
  {
    "type": "heading",
    "defaultValue": "GC Watch Config"
  },
  {
    "type": "text",
    "defaultValue": "Choose colours."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Background Colours"
      },
      {
        "type": "color",
        "messageKey": "backgroundColor",
        "defaultValue": "#0x5500AA",
        "label": "Background Bluetooth OK"
      },
      {
        "type": "color",
        "messageKey": "backgroundBTColor",
        "defaultValue": "0xFF0000",
        "label": "Background Bluetooth KO"
      },
      {
        "type": "color",
        "messageKey": "quadrantColor",
        "defaultValue": "0x55FFFF",
        "label": "Inner night"
      },
      {
        "type": "color",
        "messageKey": "innerFillColor",
        "defaultValue": "0xFFFFFF",
        "label": "Inner daylight"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Objects Colours"
      },
      {
        "type": "color",
        "messageKey": "textColor",
        "defaultValue": "0xFFFF00",
        "label": "Date and steps counter"
      },
      {
        "type": "color",
        "messageKey": "batteryColor",
        "defaultValue": "0xFF5555",
        "label": "Battery"
      },
      {
        "type": "color",
        "messageKey": "hourColor",
        "defaultValue": "0x550055",
        "label": "Text time"
      },

      {
        "type": "color",
        "messageKey": "hourhandColor",
        "defaultValue": "0x550055",
        "label": "Hours hand"
      },
      {
        "type": "color",
        "messageKey": "minhandColor",
        "defaultValue": "0xAA5500",
        "label": "Minutes hand"
      },
      {
        "type": "color",
        "messageKey": "handColor24",
        "defaultValue": "0xAA00FF",
        "label": "24 hours relative position"
      }
    ]
  },  
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Other stuff"
      },
      {
        "type": "toggle",
        "messageKey": "fontBold",
        "label": "Font bold?",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "weatherView",
        "label": "Enable weather",
        "defaultValue": true
      },
      {
        "type": "input",
        "messageKey": "API_key",
        "defaultValue": "",
        "label": "Open Weather API Key"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "save"
  }
];