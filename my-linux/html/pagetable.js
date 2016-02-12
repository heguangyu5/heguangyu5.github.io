$.fn.pagetable = function (tables) {
    var html = '';
    tables.forEach(function (table) {
        html += '<table class="table table-bordered table-condensed" style="width:150px;float:left;margin-right:20px;font-size:12px;">';
        html += '<caption>' + table.name + ' (0x' + table.addr.toString(16) + ')' + '</caption>';
        for (var i = 0; i < 512; i++) {
            html += '<tr>';
            html += '<th>' + i + '</th>';
            html += '<td>';
            if (typeof table.entries[i] === 'number') {
                html += '0x' + table.entries[i].toString(16);
            } else if (table.entries[i]) {
                html += table.entries[i];
            }
            html += '</td>';
            html += '</tr>';
        }
        html += '</table>'
    });
    html += '<div class="clearfix"></div>';
    this.html(html);
};
